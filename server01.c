#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>
#include <netdb.h>
#include "threadpool.h"
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0


int getSize(int fd)
{
	struct stat fdSize;

	fstat(fd, &fdSize);

	int size = fdSize.st_size;

	return size;
}

char *getFilename(int fd)
{
	char buffer[4096];
	ssize_t rxBytes = 0;
	char *patron1 = "GET /";
	char *patron2 = " HTTP";
	char *target = NULL;
	char *start = NULL; 
	char *end = NULL;

	rxBytes = read(fd, buffer, 4096);

	if (start = strstr(buffer, patron1))
	{
		start += strlen(patron1);

		if (end = strstr(start, patron2))
		{
			target = (char *)malloc(end - start + 1);
			memcpy(target, start, end - start);
		}
	}

	printf("%s", buffer);

	return target;
}

void archivo(void *infofile)
{
	int cliente = *((int *)infofile);
	int imagen;
	char webpage[] = "HTTP/1.1 200 OK \r\n"
					 "Content-Type: image/jpeg\r\n\r\n";
	char filename[4096];

	//Envio el header al cliente.
	write(cliente, webpage, sizeof(webpage) - 1);

	//Obtengo el nombre de la imagen recibida.
	strcpy(filename, getFilename(cliente));

	//Abro la imagen.
	imagen = open(filename, O_RDONLY);

	sleep(5);
	sendfile(cliente, imagen, NULL, getSize(imagen));

	//Cierro la conexion con el cliente.
	close(imagen);
	close(cliente);
}

int main(int argc, char *argv[])
{
	
	

	int socket_server; //File Descriptor Socket Servidor.
	int socket_client; //File Descriptor Socket Cliente.

	struct addrinfo hints; //Direccion template
	struct addrinfo *serverInfo;

	threadpool_t *pool = threadpool_create(2, 5, 0); //Pool de THREADS

	//Configuro el servidor de forma automatica
	memset(&hints, 0, sizeof(hints)); //Limpio la estructura.
	hints.ai_family = AF_INET;		  //Configuro la Familia ETHERNET
	hints.ai_socktype = SOCK_STREAM;  //Configuro para TCP
	hints.ai_flags = AI_PASSIVE;	  //Pongo el SOCKET en modo pasivo.

	//Tomo completo la direccion de forma automatica
	getaddrinfo(NULL, argv[1], &hints, &serverInfo);
	//Creo el SOCKET
	socket_server = socket(AF_INET, SOCK_STREAM, 0);
	//Enlazo el SOCKET al PUERTO
	bind(socket_server, serverInfo->ai_addr, serverInfo->ai_addrlen);
	listen(socket_server, 5);
	printf("Servidor levantado exitosamente!");
	printf("Comienzo a escuchar conexiones");

	//Comienzo a archivo los Clientes
	while (TRUE)
	{

		//Acepto conexion del cliente.
		socket_client = accept(socket_server, (struct sockaddr *)&serverInfo->ai_addr, (socklen_t *)&serverInfo->ai_addrlen);
		//Atiendo al cliente.
		threadpool_add(pool, archivo, (void *)&socket_client, 0);
		
	}

	//Libero recursos.
	threadpool_destroy(pool, 0);
	freeaddrinfo(serverInfo);

	//Cierro el programa
	return 0;
}
