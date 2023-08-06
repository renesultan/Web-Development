#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * Convenient error handler for printing errno-associated message and exit(1).
 */
static void die(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char **argv) {
	/*
	 * Check and obtain command-line arguments
	 */
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <server-addr>  <server-port> <URI> \n",
				argv[0]);
		exit(1);
	}

	char *server_ip = argv[1];
	//printf("DEBUGGING STATEMENT: server_ip: %s\n", server_ip); 
	char *server_port = argv[2];
	//printf("DEBUGGING STATEMENT: server_port: %s\n", server_port);
	char *uri = argv[3];
	//printf("DEBUGGING STATEMENT: uri: %s\n", uri);

	/*
	 * Obtain server address information using getaddrinfo().
	 */

	// Define hints for getaddrinfo(), which we need to zero out first.
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	// Specify what kind of connection we intend to make; these values tell
	// getaddrinfo() that we don't care about other kinds of addresses.
	hints.ai_family = AF_INET;        // Only accept IPv4 addresses
	hints.ai_socktype = SOCK_STREAM;  // stream socket for TCP connections
	hints.ai_protocol = IPPROTO_TCP;  // TCP protocol

	// Define where getaddrinfo() will return the information it found.
	struct addrinfo *info;

	// Call getaddrinfo(), specifying the server IP address and port as strings.
	// getaddrinfo() will parse those for us and point info to the result.
	int addr_err;
	if ((addr_err = getaddrinfo(server_ip, server_port, &hints, &info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addr_err));
		exit(1);
	}


	/*
	 * Create socket() and connect() it to server.
	 */

	// Create socket() according to the address family, socket type, and
	// protocol of the address info.  Since we specified AF_INET, SOCK_STREAM,
	// and IPPROTO_TCP in the hints, this should be equivalent to just calling
	// socket(AF_INET, SOCK_STREAM, IPPROTO_TCP).
	int serv_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (serv_fd < 0)
		die("socket");

	// Connect socket with server address; the IP address and port in
	// info->ai_addr should be the same address and port that getaddrinfo()
	// parsed from server_address and server_port.
	if (connect(serv_fd, info->ai_addr, info->ai_addrlen) < 0)
		die("connect");

	// BTW: we're done with the info retrieved by getaddrinfo(), so free it.
	freeaddrinfo(info);

	//printf("checked data, ontained server info using getaddrinfo and createdsocket successfully!\n");


	/*
	 * send file pver the connection
	 */


	//construct a file pointer to write into the server
	//using serv_fd as the underlying file descriptor
	FILE *fpw = fdopen(serv_fd, "wb");
	//error-handling
	if(fpw == NULL)
		die("fdopen(for writing)");
	//printing the HTTP Request and sending it to the server
	fprintf(fpw, "GET %s HTTP/1.0\r\nHost: %s:%s\r\n\r\n", uri, server_ip, server_port);
	//printf("GET %s HTTP/1.0\r\nHost: %s:%s\r\n\r\n", uri, server_ip, server_port);
	fflush(fpw);


	/*
	 * read the server response
	 */

	//constructing a FILE pointer to read from the server
	//we dup(serv_fd) to make sure this FILE stream uses a different
	//underlying file descriptor (referring to the same socket) 
	FILE *fpr = fdopen(dup(serv_fd), "rb");
	//error handling
	if(fpr == NULL)
		die("fdopen(for reading)");

	char buf[8192];
	char *response;
	//reading the server response
	response = fgets(buf, sizeof(buf), fpr);
	//checks if HTTP request is correct
	if(strncmp(response+9, "200", 3) !=  0){
		printf("%s\n", buf);
		exit(1);

		/*
		 * Donwload the single web page
		 */


	} else{
		//    printf("Correct HTTP request: %s\n", buf);
		while(fgets(buf, sizeof(buf), fpr)){
			//	    printf("DEBUGGING STATEMENT: what buf contains: %s\n", buf);
			if(strcmp(buf,"\r\n") == 0){
				//		    printf("DEBUGGING STATEMENT: got into the if statement for blank line");
				//	            printf("DEBUGGING STATEMENT: this is what buf contains in the if statement: buf = %s\n", buf);
				break;
			}
		}
		//   printf("DEBBUGGING STATEMENT: where are we in terms of reading the file: %s\n", fgets(buf,sizeof(buf), fpr));
		// creating a new file with the file name gotten from the URI
		char* filename;
		filename = strrchr(uri, '/');
		filename = filename + 1;
		FILE *fpd = fopen(filename, "wb");
		//printf("DEBUGGING STATEMENT: filename: %s\n", filename);

		/*
		 * write the contents of the file we want to download into the file we just created
		 */

		char download_buf[4096];
		size_t bytes_read; 
		while((bytes_read = fread(download_buf, 1, sizeof(download_buf), fpr)) > 0){
			if(fwrite(download_buf, 1, bytes_read, fpd) < 1)
				die("writing file error");
		}

		if (ferror(fpd)){
			die("reading file error");
		}

		//printf("DEBUGGING STATEMENT: Done downloading the web page\n");
		//printf("DEBUGGING STATEMENT: number of bytes read: %ld\n", bytes_read);

    		fclose(fpr);
		fclose(fpw);
		fclose(fpd);
		close(serv_fd);		

	}
	return 0;

}


