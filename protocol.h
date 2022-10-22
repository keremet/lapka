const static int MAX_HOSTNAME = 255;
const static int MAX_STATE = 2500;
void send_str(int sockfd, const char* str);
char* recv_str(int sockfd);
