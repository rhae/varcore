

#include "console.h"

#include "common.h"
#include "textio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define GET_DATA( p ) struct TELNET_DATA *d = (struct TELNET_DATA *)p;


struct TELNET_DATA {
  struct {
    int sockfd;
    int port;
    int backlog;
  } server;

  struct {
    int sockfd;
    struct sockaddr addr;
    socklen_t addr_len;
  } client;
  
};

struct TELNET_DATA data;

static int find_opt( char const *p, char const **opt_list, char **endp, int *idx ) {

  int found = 0;
  int i = 0;

  char *needle;
  char *sep;
  size_t len;

  needle = strdup( p );
  if( !needle ) {
    return 0;
  }

  sep = strchr( needle, '=');
  len = sep ? (size_t) (sep - needle) : strlen(needle);
  needle[len] = '\0';

  for( char const **opt = opt_list; *opt != NULL; opt++, i++ ) {

    if( 0 == strcmp( *opt, needle )) {
      *idx = i;
      *endp = (char*) (p+len+1);
      found = 1;
      break;
    }
  }

  free( needle );

  if( !found ) {
    *endp = (char*) p;
  }

  return found;
}

int setup( int port, int backlog ) {
  struct sockaddr_in server;
  int yes=1;
  int ret;

  int sockfd = socket(AF_INET , SOCK_STREAM , 0);
	if (sockfd == -1)
	{
		printf("Could not create socket");
    return -1;
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );

  // lose the pesky "Address already in use" error message
  ret = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
  if( ret < 0 ) {
      perror("setsockopt SO_REUSEADDR failed.");
      return -2;
  } 
	
	//Bind
  ret = bind(sockfd ,(struct sockaddr *)&server , sizeof(server));
  if( ret < 0 )
	{
		//print the error message
		perror("bind failed.");
		return -3;
	}
	
	//Listen
	ret = listen( sockfd , backlog );
  if( ret < 0 )
	{
		//print the error message
		perror("listen failed.");
		return -4;
	}
  
  return sockfd;
}

char *iptostr(char *s, size_t maxlen, const struct sockaddr *sa)
{
  switch(sa->sa_family) {
    case AF_INET:
      inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
      break;

    case AF_INET6:
      inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
      break;

    default:
      strncpy(s, "Unknown AF", maxlen);
      return NULL;
  }

  return s;
}

int telnet_open( char const *args, void *priv ) {
  GET_DATA( priv );

  enum { optPORT };
  char const *options[] = { "port", 0 };
  int option;
  int have_opt;
  char *endp;
  
  d->client.sockfd = -1;
  d->server.port = 8023;
  d->server.backlog = 5;
  have_opt = find_opt( args, options, &endp, &option );

  if( have_opt ) {
    switch( option ) {
      case optPORT:
        d->server.port = strtol( endp, 0, 0 );
        break;
    }
  }

  d->server.sockfd = setup( d->server.port, d->server.backlog );

  if( d->client.sockfd == -1 ) {
    char buf[256];
    d->client.addr_len = sizeof(struct sockaddr);
    d->client.sockfd = accept( d->server.sockfd, &d->client.addr, &d->client.addr_len );

    iptostr( buf, sizeof(buf), &d->client.addr );
    printf("connect from: ip=%s\n", buf );
  }

  return d->server.sockfd;
}

int telnet_read( char *buf, int bufsz, void *priv ) {
  GET_DATA( priv );
  
  int ret = recv( d->client.sockfd, buf, bufsz, 0 );
  if( ret > 0 ) {
    char *p = buf + ret-1;
    while( *p == '\r' || *p == '\n' ) {
      p--;
    }
    p++;
    *p = '\0';
    ret = p - buf;
  }
  return ret;
}

int telnet_write( char const *buf, int bufsz, void *priv ) {
  UNUSED_PARAM( bufsz );
  GET_DATA( priv );

  return send( d->client.sockfd, buf, bufsz, 0 );
}

int telnet_ioctl( int arg, void *argp, int argsz, void *priv ) {
  UNUSED_PARAM( arg );
  UNUSED_PARAM( argp );
  UNUSED_PARAM( argsz );
  UNUSED_PARAM( priv );
  return 0;
}

struct TEXT_IO telnet = {
  .name = "TELNET",
  .open = telnet_open,
  .read = telnet_read,
  .write = telnet_write,
  .ioctl = telnet_ioctl,
  .priv = (void *) &data
};

void telnet_init() {

  textio_register( &telnet );
}
