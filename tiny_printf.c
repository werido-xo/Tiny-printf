#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

struct printf_info {
    char *bf;   /* Digit buffer */
    char zs;    /* non-zero if a digit has been written */
    char *outstr;   /* Next output position for sprintf() */

    /* Output a character */
    void (*putc)(struct printf_info *info, char ch);
};     


static void out(struct printf_info *info, char c) 
{
    *info->bf++ = c; 
}   

static void out_dgt(struct printf_info *info, char dgt)    
{ 
	out(info, dgt + (dgt < 10 ? '0' : 'a' - 10)); 
	info->zs = 1; 
} 

static void div_out(struct printf_info *info, unsigned long *num,
							 unsigned long div) 
{ 
	unsigned char dgt = 0; 
	while (*num >= div) {    
		*num -= div;  
		dgt++; 
	}       

	if (info->zs || dgt > 0)    
		out_dgt(info, dgt); 
}

static int _vprintd(struct printf_info *info, const char *fmt, va_list va)
{
	char ch;
	char *p;
	unsigned long num;
	char buf[12];
	unsigned long div;

	while ((ch = *(fmt++))) {
		if (ch != '%') {
			info->putc(info, ch);
		} else {
			bool lz = false;
			int width = 0;
			bool islong = false;

			ch = *(fmt++);
			if (ch == '-')
				ch = *(fmt++);

			if (ch == '0') {
				ch = *(fmt++);
				lz = 1;
			}

			if (ch >= '0' && ch <= '9') {
				width = 0;
				while (ch >= '0' && ch <= '9') {
					width = (width * 10) + ch - '0';
					ch = *fmt++;
				}
			}
			if (ch == 'l') {
				ch = *(fmt++);
				islong = true;
			}

			info->bf = buf;
			p = info->bf;
			info->zs = 0;

			switch (ch) {
			case '\0':
				goto abort;
			case 'u':
			case 'd':
			case 'i':
				div = 1000000000;
				if (islong) {
					num = va_arg(va, unsigned long);
					if (sizeof(long) > 4)
						div *= div * 10;
				} else {
					num = va_arg(va, unsigned int);
				}

				if (ch != 'u') {
					if (islong && (long)num < 0) {
						num = -(long)num;
						out(info, '-');
					} else if (!islong && (int)num < 0) {
						num = -(int)num;
						out(info, '-');
					}
				}
				if (!num) {
					out_dgt(info, 0);
				} else {
					for (; div; div /= 10)
						div_out(info, &num, div);
				}
				break;
			case 'p':
				islong = true;
				/* no break */
			case 'x':
				if (islong) {
					num = va_arg(va, unsigned long);
					div = 1UL << (sizeof(long) * 8 - 4);
				} else {
					num = va_arg(va, unsigned int);
					div = 0x10000000;
				}
				if (!num) {
					out_dgt(info, 0);
				} else {
					for (; div; div /= 0x10)
						div_out(info, &num, div);
				}
				break;
			case 'c':
				out(info, (char)(va_arg(va, int)));
				break;
			case 's':
				p = va_arg(va, char*);
				break;
			case '%':
				out(info, '%');
			default:
				break;
			}

			*info->bf = 0;
			info->bf = p;
			while (*info->bf++ && width > 0)
				width--;
			while (width-- > 0)
				info->putc(info, lz ? '0' : ' ');
			if (p) {
				while ((ch = *p++))
					info->putc(info, ch);
			}
		}
	}

abort:
	return 0;
}


static void putc_normal(struct printf_info *info, char ch)
{
	// this function need instead by user
	putchar(ch);
}

int vprintf(const char *fmt, va_list va)
{
	struct printf_info info;

	info.putc = putc_normal;
	return _vprintd(&info, fmt, va);
}

int printd(const char *fmt, ...)
{
	struct printf_info info;

	va_list va;
	int ret;

	info.putc = putc_normal;
	va_start(va, fmt);
	ret = _vprintd(&info, fmt, va);
	va_end(va);

	return ret;
}


int main() {
	int avg = 2134;

    printd("Average: 0x%p \n", &avg);
    return 0;
}
