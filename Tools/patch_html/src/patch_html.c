///////////////////////////////////////////////////////////////////////////////////
// File : patch_html.c
// Contains: html patcher : <img> size definition fixer.
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void basepath(char const *path, char * out)
{
	char *s = strrchr(path, '/');
	if (!s)
	{
		strcpy(out,"./");
	}
	else
	{
		strcpy(out,path);
		out[ (s - path) + 1] = 0;
	}
}

char *basename(char const *path)
{
    char *s = strrchr(path, '/');
    if (!s)
        return strdup(path);
    else
        return strdup(s + 1);
}

int get_image_size(char * image_path, int * x, int * y)
{
	FILE *p;
	char cmd[512];
	char retstr[512];
	char *yp;

	retstr[0] = 0;

	sprintf(cmd,"convert %s -print \"%cw,%c%c\" /dev/null",image_path,'%','%','h');

	p = popen(cmd, "r");
	if(p)
	{
		while (fgets(retstr, 512, p) !=NULL);

		pclose(p);
	}

	yp = strchr(retstr,',');
	if(yp)
	{
		*yp = 0;
		yp++;

		*y = atoi(yp);
		*x = atoi(retstr);
		return 1;
	}

	return 0;
}

char * load_html(char * path)
{
	FILE * f;
	char * buf;
	int    size;

	buf = NULL;

	printf("Loading %s...\n",path);

	f = fopen(path,"r");
	if(f)
	{
		fseek(f,0,SEEK_END);
		size = ftell(f);
		fseek(f,0,SEEK_SET);

		buf = malloc(size + 1);
		if(buf)
		{
			memset(buf,0,size + 1);
			if(fread(buf,size,1,f) != 1)
			{
				free(buf);
				buf = NULL;
			}
		}
		fclose(f);
	}

	if(!buf)
		printf("Loading failed ! :-(\n");

	return buf;
}

int strcicmp(char const *a, char const *b)
{
	int d;

	for (;; a++, b++)
	{
		d = tolower((unsigned char)*a) - tolower((unsigned char)*b);

		if (d != 0 || !*a)
			return d;
	}
}

int getproperty(char * buf,char * propertyname, char * out)
{
	char tmpstr[512];
	int i;
	char *ptr2;
	int offset;

	strcpy(tmpstr,propertyname);
	strcat(tmpstr,"=\"");

	ptr2 = strstr(buf,tmpstr);
	if(ptr2)
	{
	   ptr2 = strstr(ptr2,"\"");
	   if(ptr2)
	   {
			i = 0;
			ptr2++;
			offset = ptr2 - buf;
			if(out)
			{
				while(*ptr2!='"')
				{
					out[i] = *ptr2;
					i++;
					ptr2++;
				}
				out[i] = 0;
			}
			return offset;
		}
	}

	return 0;
}

int setproperty(char * buf,char * propertyname, char * in)
{
	char tmpstr[512];
	char tmpstr2[512];
	int offset;
	int s;

	offset = getproperty(buf,propertyname, tmpstr);
	if(offset)
	{
		memset(tmpstr2,0,sizeof(tmpstr2));
		strncpy(tmpstr2,buf,offset);
		strcat(tmpstr2,in);
		strcat(tmpstr2,&buf[offset+strlen(tmpstr)]);
		strcpy(buf,tmpstr2);
	}
	else
	{
		s = strlen(buf);
		if(s)
		{
			buf[s-1] = ' ';
			sprintf(tmpstr,"%s=\"%s\">",propertyname,in);
			strcat(buf,tmpstr);
		}
	}

	return 0;
}

int find_img(int index,char * html_buf, char * bpath, int *end_offset, char * patched_element)
{
	char tmpstr[1024];
	char filename[1024];
	char tmppath[1024];
	char altstr[1024];
	char * fname;

	int i,offset,xsize,ysize;
	char * ptr;
	char * start,*end;
	char width_str[1024];
	char height_str[1024];

	ptr = html_buf;
	i = 0;
	do
	{
		ptr = strstr(ptr, "<img");
		if(ptr)
			ptr++;

		i++;
	}while(i<(index + 1) && ptr);

	start = NULL;
	end = NULL;

	if(ptr)
	{
		ptr--;
		start = ptr;

		while((*ptr != '>') && *ptr)
		{
			ptr++;
		}

		if(*ptr)
			end = ptr;
	}

	if(start && end)
	{
		offset = start - html_buf;
		*end_offset = end - html_buf;

		i = 0;
		do
		{
			tmpstr[i++] = *start;
			start++;
		}while(start != end+1);
		tmpstr[i] = 0;

		if( getproperty(tmpstr,"src", filename) )
		{
			strcpy(tmppath,bpath);
			strcat(tmppath,filename);

			printf("File path: %s\n",tmppath);
			if(get_image_size(tmppath,&xsize,&ysize))
			{
				printf("xsize : %d, ysize : %d\n",xsize,ysize);

				if(getproperty(tmpstr,"width", width_str))
				{
					if(atoi(width_str) == xsize )
					{
						printf("width property present and matching !\n");
					}
					else
					{
						printf("width property present but NOT matching : %s != %d\n",width_str,xsize);
					}
				}
				else
				{
					printf("missing width property\n");

					sprintf(width_str,"%d",xsize);
					setproperty(tmpstr,"width", width_str);
				}

				if(getproperty(tmpstr,"height", height_str))
				{
					if(atoi(height_str) == ysize )
					{
						printf("height property present and matching !\n");
					}
					else
					{
						printf("height property present but NOT matching : %s != %d\n",height_str,ysize);
					}
				}
				else
				{
					printf("missing height property\n");
					sprintf(height_str,"%d",ysize);
					setproperty(tmpstr,"height", height_str);
				}

				if(!getproperty(tmpstr,"alt", altstr))
				{
					printf("alt property not defined.\n");
					fname = basename(tmppath);
					if(fname)
					{
						setproperty(tmpstr,"alt", fname);
					}
				}
				else
				{
					if(!strlen(altstr))
					{
						printf("empty alt property\n");
						fname = basename(tmppath);
						if(fname)
						{
							setproperty(tmpstr,"alt", fname);
							free(fname);
						}
					}
				}
			}

			strcpy(patched_element,tmpstr);

			return offset;
		}
	}

	return 0;
}

int patch_html_page(char * path)
{
	FILE *f;
	char * html_buf;
	int  i,offset,prev_offset,end_offset;
	char bpath[512];
	char ppath[512];
	char patched_element[512];

	html_buf = load_html(path);
	if(html_buf)
	{
		basepath(path, bpath);

		strcpy(ppath,path);
		strcat(ppath,"_patched");

		f = fopen(ppath,"wb");
		if(f)
		{
			prev_offset = 0;
			patched_element[0] = 0;
			end_offset = 0;
			offset = 0;
			i = 0;
			while(offset = find_img(i,html_buf,bpath,&end_offset,patched_element), offset!=0)
			{
				fwrite(&html_buf[prev_offset],(offset-prev_offset),1,f);
				fwrite(patched_element,strlen(patched_element),1,f);

				prev_offset = end_offset + 1;
				i++;
				patched_element[0] = 0;
			}

			fwrite(&html_buf[prev_offset],strlen(&html_buf[prev_offset]),1,f);

			fclose(f);
		}

		free(html_buf);

		return 1;
	}

	return -1;
}

int main (int argc, char ** argv)
{
	int i;

	if(argc>1)
	{
		for(i=1;i<argc;i++)
		{
			patch_html_page(argv[i]);
		}
	}
	else
	{
		printf("Syntax : %s in_html_files\n",argv[0]);
	}

	exit(1);
}
