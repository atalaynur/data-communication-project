#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //close() ve diğer UNIX fonksiyonları için.
#include <sys/socket.h> //Soket oluşturma ve yönetme işlemleri için.
#include <arpa/inet.h> //IP adresi dönüştürme ve bağlantı ayarları için.
#include <time.h>// Rastgele sayı üretimi için zaman fonksiyonları.

#define PORT 12345
#define MAX_BUFFER_SIZE 1024

// Kazananı belirleme fonksiyonu
int determine_winner(char player_choice, char server_choice) {
    if (player_choice == server_choice) return 0; // Beraberlik
    if ((player_choice == 'T' && server_choice == 'M') || 
        (player_choice == 'K' && server_choice == 'T') || 
        (player_choice == 'M' && server_choice == 'K')) {
        return 1; // Oyuncu kazandı
    }
    return -1; // Sunucu kazandı
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];
    int player_score = 0, server_score = 0; // Skorlar
    
    srand(time(NULL)); // Rastgele sayı

/*Soket oluşturma
-socket(): Sunucu için bir TCP soketi oluşturur.
-AF_INET: IPv4 kullanımı.
-SOCK_STREAM: TCP protokolü.*/

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Soket oluşturulamadı");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT)

/*Soket bağlama
-server_addr: Sunucunun adres bilgilerini ayarlar:
-INADDR_ANY: Sunucunun herhangi bir IP adresinden gelen bağlantıları kabul etmesini sağlar.
-htons(PORT): Port numarasını ağ bayt sırasına çevirir.
-bind(): Soketi belirtilen adrese bağlar.*/

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bağlantı hatası");
        exit(EXIT_FAILURE);
    }

    /* Sunucuyu dinlemeye alma
-listen(): Sunucuyu bağlantılar için dinleme moduna geçirir.
-3: Maksimum 3 istemci kuyruğu uzunluğu.*/

    if (listen(server_fd, 3) < 0) {
        perror("Dinleme hatası");
        exit(EXIT_FAILURE);
    }
    printf("Sunucu dinliyor...\n");

    /*Bağlantıyı kabul etme
-accept(): İstemciden gelen bağlantıyı kabul eder ve istemci için yeni bir soket oluşturur.*/

    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("Bağlantı hatası");
        exit(EXIT_FAILURE);
    }
    printf("İstemci bağlandı\n");

    // Oyun başlatma
    while (player_score < 2 && server_score < 2) {
        memset(buffer, 0, MAX_BUFFER_SIZE);

        // İstemciye seçim mesajı gönder
        send(client_fd, "HAZIRSAN! ", 
             strlen("Taş (T), Kağıt (K) veya Makas (M) seçin:"), 0);

        // İstemciden veri al
        int bytes_received = recv(client_fd, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            perror("İstemciden veri alınamadı");
            break;
        }

        buffer[bytes_received] = '\0'; // Gelen veriyi sonlandır
        printf("\nİstemcinin seçimi: %c\n", buffer[0]);

        // Geçerli bir seçim kontrolü
        if (buffer[0] != 'T' && buffer[0] != 'K' && buffer[0] != 'M') {
            send(client_fd, "Geçersiz seçim! Taş (T), Kağıt (K) veya Makas (M) seçin.\n", 55, 0);
            continue; // Yeni seçim bekle
        }

        // Sunucunun seçim yapması
        char server_choice = "TKM"[rand() % 3];
        printf("Sunucunun seçimi: %c\n", server_choice);

        // Kazananı belirle
        int result = determine_winner(buffer[0], server_choice);

        if (result == 1) {
            player_score++;
        } else if (result == -1) {
            server_score++;
        }

        // Skor bilgisini gönder
        snprintf(buffer, MAX_BUFFER_SIZE, "Sunucu seçimi: %c. Skor: Siz: %d - Sunucu: %d\n", 
                 server_choice, player_score, server_score);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // Oyun sonu mesajı gönder
    memset(buffer, 0, MAX_BUFFER_SIZE);
    if (player_score == 2) {
        snprintf(buffer, MAX_BUFFER_SIZE, "Tebrikler! Kazandınız! \n\nSon Skor: Siz: %d - Sunucu: %d\n", 
                 player_score, server_score);
    } else {
        snprintf(buffer, MAX_BUFFER_SIZE, "Üzgünüz, kaybettiniz. \n\nSon Skor: Siz: %d - Sunucu: %d\n", 
                 player_score, server_score);
    }
    send(client_fd, buffer, strlen(buffer), 0);

    // Bağlantıları kapatma
    close(client_fd);
    close(server_fd);
    return 0;
}

