//Modified by LiuQun
//Email: liuqun68@gmail.com
//Thanks to thorqq and AGanNo2!


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pcap.h>
#include<libnet.h>
#include<stdbool.h> // C99 standard bool
#include<assert.h>

/* �Ӻ������� */
int Authentication(const char *UserName, const char *Password, const char *DeviceName);

int main(int argc, char* argv[])
{
    pcap_if_t *d;
    int i;
    int iMax;
    int inum;
    char *UserName;
    char *Password;
    char DeviceName[128];
    const size_t DEVICE_NAME_MAX=sizeof(DeviceName)-1;

    if (argc >= 3)
    {
        UserName = argv[1];
        Password = argv[2];
        fprintf(stderr, "USERNAME=%s\n", UserName);
        fprintf(stderr, "PASSWORD=%s\n", Password);
    }

    /* ��������в�����ʽ */
    if (argc<3)
    {
        fprintf(stderr, "�����в�������\n");
        fprintf(stderr,	"��ȷ�ĸ�ʽΪ��\n");
        fprintf(stderr,	"    %s username password [netcard]\n", argv[0]);
        fprintf(stderr, "(ע����ָ������ʱ����ʾ�û��ֶ�ѡ�񣬵��û����������Ǳ�������������е�)\n");/*TODO: �����д��Ľ�*/
        exit(-1);
    }
    else if (argc == 4 && argv[3] != NULL)
    {/* ��������ָ�������ӿ��豸�� */
        size_t n = strlen(argv[3]);
        if (DEVICE_NAME_MAX < n)
        {   n = DEVICE_NAME_MAX;
        }
        memcpy(DeviceName, argv[3], n);
        DeviceName[n] = '\0';
        /* FIXME: ����û�м���û�ָ�������������Ƿ���ڣ��Լ��ܷ�libnet��libpcap�򿪲����в��� */
        /* TODO: CheckDevice(DeviceName); */
        fprintf(stderr, "NETCARD=%s\n", DeviceName);
    } else
    {/* ��ָ������ʱ����ʾ�û��ֶ�ѡ�� */
        pcap_if_t *alldevs;
        char errbuf[PCAP_ERRBUF_SIZE];
        /* ��ȡ�豸�б�(ͨ��libpcap) */
        if (pcap_findalldevs(&alldevs, errbuf) == -1)
        {
            fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
            exit(1);
        }

        fprintf(stderr, "NETCARD=\n");
        d = alldevs;
        i = iMax = 0;
        while (d != NULL)
        {
            if (d->name && strcmp(d->name, "lo")!=0 && strcmp(d->name, "any")!=0 && strcmp(d->name, "")!=0)
            {
                i++;
                printf("%d.(%s)\n", i, d->name);
            }
            d = d->next;
        }
        iMax = i;
        if(iMax==0)
        {
            fprintf(stderr, "\nNo interfaces found! Make sure libpcap is installed and you are root.\n");
            exit(1);
        }
        printf("\nPlease choose net card (1-%d):",iMax);
        inum = 0;
        scanf("%d", &inum);
        while (inum<=0 || inum>iMax)
        {
            int ch;
            fprintf(stderr, "Error: Unexpected input!\n");
            if (!feof(stdin))
            {   do { /* ���stdin��������ʣ����ַ�, ͨ����scanf��ȡ�������Ļس���'\n' */
                    ch = fgetc(stdin);
                   } while (ch != EOF && ch != '\n');
            }
            printf("Please choose net card (1-%d):",iMax);
            inum = 0;
            scanf("%d", &inum);
        }

        i = 1;
        d = alldevs;
        while (i < inum)
        {
            if (d->name && strcmp(d->name, "lo")!=0 && strcmp(d->name, "any")!=0 && strcmp(d->name, "")!=0)
            {
                i++;
            }
            d=d->next;
        }
        size_t n = strlen(d->name);
        if (n > DEVICE_NAME_MAX)
        {   n = DEVICE_NAME_MAX;
        }
        memcpy(DeviceName, d->name, n);
        DeviceName[n] = '\0';
        printf("%d.(%s)\n", inum, DeviceName);
        pcap_freealldevs(alldevs);
    }

    /* �����Ӻ������802.1X��֤ */
    Authentication(UserName, Password, DeviceName);
    return(0);
}

