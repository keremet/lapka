const static int MAX_HOSTNAME = 255;
const static int MAX_STDOUT = 1000;
const static int MAX_STDERR = 1000;
const static int MAX_STATE = MAX_STDOUT + MAX_STDERR + 500;
void send_str(int sockfd, const char* str);
//char* recv_str(int sockfd);
