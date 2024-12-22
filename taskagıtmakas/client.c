#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // UNIX tabanlı sistemler için sistem çağrılarını
#include <sys/socket.h> //Ağ programlaması için socket (soket) işlemleri sağlar.
#include <arpa/inet.h> //Ağ programlamasında IP adresi dönüşümü ve işlemleri için kullanılır.

#define PORT 12345
#define MAX_BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    // Soket oluşturma
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Soket oluşturulamadı");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Sunucu adresine bağlanma
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Geçersiz adres");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bağlantı hatası");
        exit(EXIT_FAILURE);
    }

    // Oyun başlatma
    while (1) {
        memset(buffer, 0, MAX_BUFFER_SIZE);

        // Sunucudan mesaj al
        int bytes_received = recv(sock, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            perror("Sunucudan mesaj alınamadı");
            break;
        }
        buffer[bytes_received] = '\0'; // Alınan veriyi sonlandır
        printf("%s", buffer);

        // Oyun bittiğinde çık
        if (strstr(buffer, "Tebrikler") || strstr(buffer, "Üzgünüz")) {
            break;
        }

        // Kullanıcıdan girdi al
        memset(buffer, 0, MAX_BUFFER_SIZE);
        printf("Taş (T), Kağıt (K) veya Makas (M) seçin:");
        fgets(buffer, sizeof(buffer), stdin);

        // Fazla '\n' karakterini temizle
        buffer[strcspn(buffer, "\n")] = '\0';

        // Geçersiz seçim yapılırsa yeni seçim iste
        if (buffer[0] != 'T' && buffer[0] != 'K' && buffer[0] != 'M') {
            printf("Geçersiz giriş! Taş (T), Kağıt (K) veya Makas (M) seçin.\n");
            continue;
        }

        // Sunucuya seçimi gönder
        send(sock, buffer, strlen(buffer), 0);
    }

    // Bağlantıyı kapat
    close(sock);
    return 0;
}
