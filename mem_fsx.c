/*
    `mem_fsx.c' -- very basic "/dev/mem" file system extension for DJGPP v2

    Copyright (C) 2008-2009  Robert Riebisch <rr@bttr-software.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*
    Implementation notes:
      - Access is limited to the 1st megabyte, because dosmemget() /
        dosmemput() are used.
      - No range checking is performed.
      - We support functions __FSEXT_open, __FSEXT_llseek, __FSEXT_read,
        __FSEXT_write, and __FSEXT_close.
*/

#ifdef __DJGPP__
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/fsext.h>

#ifdef DEBUG_MEM_FSX
# include <conio.h>
# define __UNUSED(x) x
#else
# define __UNUSED(x) __UNUSED_ ## x __attribute__((unused))
#endif  /* DEBUG_MEM_FSX */

static int
__mem_handler(__FSEXT_Fnumber func, int *rv, va_list args)
{
  static offset_t mem_offset;

  switch(func) {
    case __FSEXT_open:
      {
        const char *path           = va_arg(args, const char *);
        const int __UNUSED(attrib) = va_arg(args, const int);

#ifdef DEBUG_MEM_FSX
        cprintf("DEBUG_MEM_FSX: open");
        cprintf(" - path = \"%s\", attrib = %i\r\n", path, attrib);
#endif

        if (strcmp(path, "/dev/mem"))
          return 0;

        mem_offset = 0;
        *rv = __FSEXT_alloc_fd(__mem_handler);
        return 1;
      }
    case __FSEXT_llseek:
      {
        const int __UNUSED(fd) = va_arg(args, const int);
        const offset_t offset  = va_arg(args, const offset_t);
        const int whence       = va_arg(args, const int);

#ifdef DEBUG_MEM_FSX
        cprintf("DEBUG_MEM_FSX: llseek");
        cprintf(" - fd = %i, offset = %lli, whence = %i\r\n", fd, offset, whence);
#endif

        switch(whence) {
          case SEEK_SET:
            {
              *rv = mem_offset = offset;
              break;
            }
          case SEEK_CUR:
            {
              *rv = mem_offset += offset;
              break;
            }
          case SEEK_END:
            {
              *rv = mem_offset = 0xFFFFF + offset;
              break;
            }
          default:
            {
              errno = EINVAL;
              *rv = -1;
              break;
            }
        }
        return 1;
      }
    case __FSEXT_read:
      {
        const int __UNUSED(fd) = va_arg(args, const int);
        void *buffer           = va_arg(args, void *);
        const size_t length    = va_arg(args, const size_t);

#ifdef DEBUG_MEM_FSX
        cprintf("DEBUG_MEM_FSX: read");
        cprintf(" - fd = %i, length = %li\r\n", fd, length);
#endif

        dosmemget(mem_offset, length, buffer);
        mem_offset += length;
        *rv = length;
        return 1;
      }
    case __FSEXT_write:
      {
        const int __UNUSED(fd) = va_arg(args, const int);
        const void *buffer     = va_arg(args, const void *);
        const size_t length    = va_arg(args, const size_t);

#ifdef DEBUG_MEM_FSX
        cprintf("DEBUG_MEM_FSX: write");
        cprintf(" - fd = %i, length = %li\r\n", fd, length);
#endif

        dosmemput(buffer, length, mem_offset);
        mem_offset += length;
        *rv = length;
        return 1;
      }
    case __FSEXT_close:
      {
        const int fd = va_arg(args, const int);

#ifdef DEBUG_MEM_FSX
        cprintf("DEBUG_MEM_FSX: close");
        cprintf(" - fd = %i\r\n", fd);
#endif

        __FSEXT_set_function(fd, NULL);
        *rv = _close(fd);
        return 1;
      }
    default:
      {
#ifdef DEBUG_MEM_FSX
        cprintf("DEBUG_MEM_FSX: unknown function = 0x%x\r\n", func);
#endif
        break;
      }
  }
  return 0;
}

/* install handler before main() */
static void __attribute__((constructor))
djgpp_mem_startup(void)
{
  __FSEXT_add_open_handler(__mem_handler);
}

#ifdef TEST
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
  int file;
  int i;
  char c;
  char buf[10];

  file = open("/dev/mem", O_RDONLY | O_BINARY);

  printf("SEEK_SET: ");
  lseek(file, 0xFFFF5, SEEK_SET);
  for (i = 0; i <= 7; i++) {
    read(file, &c, 1);
    printf("%c", c);
  }

  printf("\nSEEK_CUR: ");
  lseek(file, -8, SEEK_CUR);
  for (i = 0; i <= 7; i++) {
    read(file, &c, 1);
    printf("%c", c);
  }

  printf("\nSEEK_END: ");
  lseek(file, -10, SEEK_END);
  for (i = 0; i <= 7; i++) {
    read(file, &c, 1);
    printf("%c", c);
  }

  close(file);

  file = open("/dev/mem", O_WRONLY | O_BINARY);
  lseek(file, 0xB8000 + (12 * 80 * 2) + (37 * 2), SEEK_SET);
  buf[0] = 'h';
  buf[1] = 0x70;
  buf[2] = 'e';
  buf[3] = 0x71;
  buf[4] = 'l';
  buf[5] = 0x72;
  buf[6] = 'l';
  buf[7] = 0x73;
  buf[8] = 'o';
  buf[9] = 0x74;
  write(file, buf, sizeof(buf));

  lseek(file, (-5 + 80) * 2, SEEK_CUR);
  buf[0] = 'w';
  buf[1] = 0x15;
  buf[2] = 'o';
  buf[3] = 0x26;
  buf[4] = 'r';
  buf[5] = 0x37;
  buf[6] = 'l';
  buf[7] = 0x48;
  buf[8] = 'd';
  buf[9] = 0x59;
  for (i = 0; i < sizeof(buf); i++) {
    write(file, &buf[i], 1);
  }
  write(file, "!\152", 2);
  close(file);

  return 0;
}
#endif  /* TEST */

#endif  /* __DJGPP__ */
