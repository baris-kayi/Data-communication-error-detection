#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT_CLIENT1 8080
#define PORT_CLIENT2 8081
#define BUFFER_SIZE 2048

// Bit flip
void inject_bit_flip(char *data) {
    int len = strlen(data);
    if (len == 0) return;
    
    int pos = rand() % len;
    int bit = rand() % 8;
    
    data[pos] ^= (1 << bit);
    printf("   [Hata Enjekte Edildi] Bit Flip: Pozisyon %d, Bit %d\n", pos, bit);
}

// Karakter değiştirme
void inject_char_substitution(char *data) {
    int len = strlen(data);
    if (len == 0) return;
    
    int pos = rand() % len;
    char old_char = data[pos];
    char new_char = 'A' + (rand() % 26);
    
    data[pos] = new_char;
    printf("   [Hata Enjekte Edildi] Karakter Degistirildi: '%c' -> '%c' (Pozisyon %d)\n", 
           old_char, new_char, pos);
}

// Karakter silme
void inject_char_deletion(char *data) {
    int len = strlen(data);
    if (len <= 1) return;
    
    int pos = rand() % len;
    char deleted = data[pos];
    
    memmove(&data[pos], &data[pos + 1], len - pos);
    printf("   [Hata Enjekte Edildi] Karakter Silindi: '%c' (Pozisyon %d)\n", deleted, pos);
}

// Karakter ekleme
void inject_char_insertion(char *data) {
    int len = strlen(data);
    if (len >= 255) return;
    
    int pos = rand() % (len + 1);
    char new_char = 'X';
    
    memmove(&data[pos + 1], &data[pos], len - pos + 1);
    data[pos] = new_char;
    printf("   [Hata Enjekte Edildi] Karakter Eklendi: '%c' (Pozisyon %d)\n", new_char, pos);
}

// Karakter yer değiştirme
void inject_char_swapping(char *data) {
    int len = strlen(data);
    if (len < 2) return;
    
    int pos = rand() % (len - 1);
    char temp = data[pos];
    data[pos] = data[pos + 1];
    data[pos + 1] = temp;
    
    printf("   [Hata Enjekte Edildi] Karakter Yer Degistirme: Pozisyon %d <-> %d\n", pos, pos + 1);
}

// Burst error
void inject_burst_error(char *data) {
    int len = strlen(data);
    if (len < 3) return;
    
    int burst_len = 3 + (rand() % 6);
    if (burst_len > len) burst_len = len;
    
    int start = rand() % (len - burst_len + 1);
    
    for (int i = 0; i < burst_len; i++) {
        data[start + i] = 'X' + (rand() % 3);
    }
    
    printf("   [Hata Enjekte Edildi] Burst Error: %d karakter bozuldu (Baslangic: %d)\n", 
           burst_len, start);
}

// Rastgele hata enjekte et
void inject_error(char *packet) {
    char data[256], method[32], control[64];
    char *token = strtok(packet, "|");
    strcpy(data, token);
    token = strtok(NULL, "|");
    strcpy(method, token);
    token = strtok(NULL, "|");
    strcpy(control, token);
    
    printf("\n--- Orijinal Veri: %s ---\n", data);
    
    int error_type = 1 + (rand() % 6);
    
    printf("   Hata Tipi: ");
    switch(error_type) {
        case 1:
            printf("Bit Flip\n");
            inject_bit_flip(data);
            break;
        case 2:
            printf("Karakter Degistirme\n");
            inject_char_substitution(data);
            break;
        case 3:
            printf("Karakter Silme\n");
            inject_char_deletion(data);
            break;
        case 4:
            printf("Karakter Ekleme\n");
            inject_char_insertion(data);
            break;
        case 5:
            printf("Karakter Yer Degistirme\n");
            inject_char_swapping(data);
            break;
        case 6:
            printf("Burst Error\n");
            inject_burst_error(data);
            break;
    }
    
    printf("--- Bozulmus Veri: %s ---\n\n", data);
    
    sprintf(packet, "%s|%s|%s", data, method, control);
}

int main() {
    WSADATA wsa;
    SOCKET server_sock1, server_sock2, client1_sock, client2_sock;
    struct sockaddr_in server_addr1, server_addr2, client_addr;
    int addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    srand(time(NULL));
    
    printf("\n=== SERVER - ARACI VE HATA ENJEKTORU ===\n\n");
    
    // Winsock başlat
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Winsock baslatilamadi. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }
    
    // Socket 1 oluştur (Client1 için)
    server_sock1 = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr1.sin_family = AF_INET;
    server_addr1.sin_addr.s_addr = INADDR_ANY;
    server_addr1.sin_port = htons(PORT_CLIENT1);
    
    bind(server_sock1, (struct sockaddr*)&server_addr1, sizeof(server_addr1));
    listen(server_sock1, 1);
    
    // Socket 2 oluştur (Client2 için)
    server_sock2 = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_addr.s_addr = INADDR_ANY;
    server_addr2.sin_port = htons(PORT_CLIENT2);
    
    bind(server_sock2, (struct sockaddr*)&server_addr2, sizeof(server_addr2));
    listen(server_sock2, 1);
    
    printf("Server baslatildi.\n");
    printf("Port %d - Client1'i bekliyor...\n", PORT_CLIENT1);
    printf("Port %d - Client2'yi bekliyor...\n\n", PORT_CLIENT2);
    
    // Client2'yi bekle
    printf("Client2 baglantisi bekleniyor...\n");
    client2_sock = accept(server_sock2, (struct sockaddr*)&client_addr, &addr_len);
    printf("Client2 baglandi!\n\n");
    
    // Client1'i bekle
    printf("Client1 baglantisi bekleniyor...\n");
    client1_sock = accept(server_sock1, (struct sockaddr*)&client_addr, &addr_len);
    printf("Client1 baglandi!\n");
    
    // Client1'den veri al
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client1_sock, buffer, BUFFER_SIZE, 0);
    
    if (bytes_received > 0) {
        printf("\nClient1'den paket alindi: %s\n", buffer);
        
        inject_error(buffer);
        
        send(client2_sock, buffer, strlen(buffer), 0);
        printf("Bozulmus paket Client2'ye gonderildi.\n");
    }
    
    closesocket(client1_sock);
    closesocket(client2_sock);
    closesocket(server_sock1);
    closesocket(server_sock2);
    WSACleanup();
    
    printf("\nServer kapatiliyor...\n");
    
    return 0;
}