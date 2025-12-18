#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 2048

// Parity hesaplama fonksiyonu
char calculate_parity(const char *data) {
    int ones_count = 0;
    
    for (int i = 0; data[i] != '\0'; i++) {
        unsigned char c = data[i];
        while (c) {
            ones_count += c & 1;
            c >>= 1;
        }
    }
    
    return (ones_count % 2 == 0) ? '0' : '1';
}

// CRC-16 hesaplama
unsigned short calculate_crc16(const char *data) {
    unsigned short crc = 0xFFFF;
    
    for (int i = 0; data[i] != '\0'; i++) {
        crc ^= (unsigned char)data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    
    return crc;
}

// Checksum hesaplama
unsigned short calculate_checksum(const char *data) {
    unsigned int sum = 0;
    
    for (int i = 0; data[i] != '\0'; i++) {
        sum += (unsigned char)data[i];
        if (sum > 0xFFFF)
            sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return (unsigned short)(~sum);
}

// Hamming kod hesaplama
void calculate_hamming(const char *data, char *result) {
    unsigned int hash = 0;
    for (int i = 0; data[i] != '\0'; i++) {
        hash = hash * 31 + (unsigned char)data[i];
    }
    sprintf(result, "%04X", (unsigned short)(hash & 0xFFFF));
}

// 2D Parity hesaplama
void calculate_2d_parity(const char *data, char *result) {
    int len = strlen(data);
    int row_parity = 0, col_parity = 0;
    
    for (int i = 0; i < len; i++) {
        unsigned char c = data[i];
        int ones = 0;
        while (c) {
            ones += c & 1;
            c >>= 1;
        }
        row_parity ^= (ones % 2);
        col_parity ^= data[i];
    }
    
    sprintf(result, "%02X%02X", row_parity, col_parity & 0xFF);
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char data[256];
    char packet[BUFFER_SIZE];
    int method;
    char control_info[64];
    
    printf("\n=== CLIENT 1 - VERI GONDERICISI ===\n\n");
    
    // Winsock başlat
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Winsock baslatılamadı. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }
    
    // Kullanıcıdan veri al
    printf("Gondermek istediginiz metni girin: ");
    fgets(data, sizeof(data), stdin);
    data[strcspn(data, "\n")] = 0;
    
    // Yöntem seç
    printf("\nHata Tespit Yontemi Secin:\n");
    printf("1. Parity Bit\n");
    printf("2. 2D Parity\n");
    printf("3. CRC-16\n");
    printf("4. Hamming Code\n");
    printf("5. Checksum\n");
    printf("Seciminiz (1-5): ");
    scanf("%d", &method);
    
    // Kontrol bilgisini hesapla
    char method_name[32];
    switch(method) {
        case 1: {
            char parity = calculate_parity(data);
            sprintf(control_info, "%c", parity);
            strcpy(method_name, "PARITY");
            printf("\nHesaplanan Parity Bit: %c\n", parity);
            break;
        }
        case 2: {
            calculate_2d_parity(data, control_info);
            strcpy(method_name, "2D_PARITY");
            printf("\nHesaplanan 2D Parity: %s\n", control_info);
            break;
        }
        case 3: {
            unsigned short crc = calculate_crc16(data);
            sprintf(control_info, "%04X", crc);
            strcpy(method_name, "CRC16");
            printf("\nHesaplanan CRC-16: %s\n", control_info);
            break;
        }
        case 4: {
            calculate_hamming(data, control_info);
            strcpy(method_name, "HAMMING");
            printf("\nHesaplanan Hamming: %s\n", control_info);
            break;
        }
        case 5: {
            unsigned short checksum = calculate_checksum(data);
            sprintf(control_info, "%04X", checksum);
            strcpy(method_name, "CHECKSUM");
            printf("\nHesaplanan Checksum: %s\n", control_info);
            break;
        }
        default:
            printf("Gecersiz secim!\n");
            WSACleanup();
            return 1;
    }
    
    // Paketi oluştur
    sprintf(packet, "%s|%s|%s", data, method_name, control_info);
    printf("\nGonderilen Paket: %s\n", packet);
    
    // Socket oluştur
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket olusturulamadi. Hata Kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // Server adresini ayarla
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    // Server'a bağlan
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Server'a baglanilamadi. Hata Kodu: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    
    printf("Server'a baglandi.\n");
    
    // Paketi gönder
    send(sock, packet, strlen(packet), 0);
    printf("Paket gonderildi!\n");
    
    closesocket(sock);
    WSACleanup();
    printf("\nClient1 kapatiliyor...\n");
    
    return 0;
}