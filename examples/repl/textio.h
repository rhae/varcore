
#pragma once

struct TEXT_IO {
  struct TEXT_IO *next;

  char const *name;
  int (*open)( char const *, void* );
  int (*read)( char *, int, void * );
  int (*write)( char const *, int, void * );
  int (*ioctl)( int, void *, int, void * );

  void *priv;
};

int textio_open( char const *, char const *);
int textio_read( char *, int );
int textio_write( char const *, int );
int textio_ioctrl( int, void *, int );

int textio_register( struct TEXT_IO * );
