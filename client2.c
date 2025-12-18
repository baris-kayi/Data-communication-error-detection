#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8081
#define BUFFER_SIZE 2048

// Parity hesaplama
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
    char buffer[BUFFER_SIZE];
    char data[256], method[32], received_control[64], computed_control[64];
    
    printf("\n=== CLIENT 2 - VERI ALICI VE KONTROL ===\n\n");
    
    // Winsock başlat
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Winsock baslatilamadi. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }
    
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
    printf("Server'a baglaniliyor...\n");
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Server'a baglanilamadi. Hata Kodu: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    
    printf("Server'a baglandi.\n");
    printf("Paket bekleniyor...\n\n");
    
    // Paketi al
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
    
    if (bytes_received > 0) {
        printf("Paket alindi: %s\n\n", buffer);
        
        // Paketi parse et
        char *token = strtok(buffer, "|");
        strcpy(data, token);
        token = strtok(NULL, "|");
        strcpy(method, token);
        token = strtok(NULL, "|");
        strcpy(received_control, token);
        
        // Kontrol bilgisini yeniden hesapla
        if (strcmp(method, "PARITY") == 0) {
            char parity = calculate_parity(data);
            sprintf(computed_control, "%c", parity);
        }
        else if (strcmp(method, "2D_PARITY") == 0) {
            calculate_2d_parity(data, computed_control);
        }
        else if (strcmp(method, "CRC16") == 0) {
            unsigned short crc = calculate_crc16(data);
            sprintf(computed_control, "%04X", crc);
        }
        else if (strcmp(method, "HAMMING") == 0) {
            calculate_hamming(data, computed_control);
        }
        else if (strcmp(method, "CHECKSUM") == 0) {
            unsigned short checksum = calculate_checksum(data);
            sprintf(computed_control, "%04X", checksum);
        }
        
        // Sonuçları göster
        printf("========================================\n");
        printf("Received Data        : %s\n", data);
        printf("Method               : %s\n", method);
        printf("Sent Check Bits      : %s\n", received_control);
        printf("Computed Check Bits  : %s\n", computed_control);
        printf("----------------------------------------\n");
        
        // Karşılaştır
        if (strcmp(received_control, computed_control) == 0) {
            printf("Status               : DATA CORRECT\n");
        } else {
            printf("Status               : DATA CORRUPTED\n");
        }
        printf("========================================\n");
    }
    
    closesocket(sock);
    WSACleanup();
    printf("\nClient2 kapatiliyor...\n");
    
    return 0;
}