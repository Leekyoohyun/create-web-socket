#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

// 확장자에 따라 Content-Type을 결정
const char* get_content_type(const char* filename) {
    char *ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream"; // 확장자 없을 경우

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".pdf") == 0) return "application/pdf";
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    return "application/octet-stream"; // 기본값
}

// 클라이언트에 파일 응답 전송
void send_response(int client_fd, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        char error_msg[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        write(client_fd, error_msg, strlen(error_msg));
        return;
    }

    // Content-Type 결정
    const char *content_type = get_content_type(filename);

    // HTTP 응답 헤더 작성
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             content_type);
    write(client_fd, header, strlen(header));

    // 파일 내용을 읽고 클라이언트에 전송
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(client_fd, buffer, bytes_read);
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("사용법: %s <포트번호>\n", argv[0]);
        return 1;
    }

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int port = atoi(argv[1]);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("소켓 생성 실패");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("바인드 실패");
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        perror("리스닝 실패");
        return 1;
    }

    printf("✅ 웹 서버가 포트 %d에서 실행 중...\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("클라이언트 연결 실패");
            continue;
        }

        printf("📥 클라이언트 접속\n");

        memset(buffer, 0, BUFFER_SIZE);
        read(client_fd, buffer, BUFFER_SIZE - 1);

        printf("📌 요청 메시지:\n%s\n", buffer);

        // 요청 파일 이름 추출
        char filename[256] = "index.html";
        if (sscanf(buffer, "GET /%s ", filename) != 1) {
            strcpy(filename, "index.html");
        }

        // GET / 만 요청한 경우 → index.html로 처리
        if (strcmp(filename, "/") == 0) {
            strcpy(filename, "index.html");
        }

        send_response(client_fd, filename);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
