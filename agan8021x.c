//Orignal coded by AGanNo2
//Modified by LiuQun
//Support site:http://AGanNo2.ys168.com
//Email: AGanNo2@163.com
//Email: liuqun68@gmail.com
//Thanks for thorqq and!


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/times.h>
#include<pcap.h>
#include<libnet.h>
#include<openssl/md5.h> // ����openssl�ṩ��MD5���� (LiuQun 2013-1-26)
#include<stdbool.h> // C99 standard bool
#include<assert.h>

typedef struct UserData
{
    char username[20];
    char password[20];
    unsigned char Ip[4];
    unsigned char Mac[6];
    char nic[60];
}USERDATA;

//ȫ������stUserData
USERDATA stUserData;

char strIP[16];             //IP��ַ
bool online = false;        // �Ƿ����ߵı��
bool FirstPacket = true;    // ���ڱ�ǵ�һ�����Ƿ��Ѿ�����

bool bVersionAFirstRequest = true;

char Passwd[64] = {0};      // ����û������� 
u_int PasswdLen = 0;        // �����ַ�������(���Ǵ洢����)

pcap_t *fp = NULL;			// �����豸
FILE *file;

// �ж����ְ������͵ı�־ 
u_char PType[4] = {0x88, 0x8e, 0x01, 0x00}; 
u_char SessionFlagA[3] = {0x00, 0x05, 0x01}; 
u_char SessionFlagB[3] = {0x00, 0x05, 0x01};
u_char SessionFlagC[3] = {0x00, 0x05, 0x14};
u_char UnknowFlagA[3] = {0x00, 0x05, 0x02}; 
u_char RequestPwdFlagA[3] = {0x00, 0x16, 0x01}; 
u_char RequestPwdFlagB[3] = {0x00, 0x16, 0x04}; 
u_char SuccessFlagA[3] = {0x00, 0x04, 0x03}; 
u_char SuccessFlagB[3] = {0x00, 0x04, 0x00}; 
u_char ByeFlagA[3] = {0x00, 0x06, 0x04}; 
u_char ByeFlagB[3] = {0x00, 0x07, 0x08};

// �����ǳ�ʼ�ĸ��ְ� 
u_char ConnectBuf[60] = {        // ��������İ�(����Ŀ��Ϊ��Ϊ���鲥��ַ01-80-c2-00-00-03) 
    0x01, 0x80, 0xc2, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x01, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
}; 

u_char DisconnectBuf[60] = {    // �Ͽ�����İ�����ͬ�İ汾�᲻ͬ
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x02, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00     
}; 
/*
0246�İ�
u_char UnknowBufA[67] = {        // δ֪���͵İ�,��ͬ�İ汾�᲻ͬ
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00, 
0x00, 0x31, 0x02, 0x01, 0x00, 0x31, 0x02, 0x01,
0x16, 0x2e, 0x25, 0x4d, 0x3b, 0x5f, 0x43, 0x5f,  
0x5d, 0x40, 0x5d, 0x5f, 0x5e, 0x5c, 0x6d, 0x6d, 
0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x02, 0x16, 0x5f,  
0x59, 0x55, 0x5e, 0xbb, 0x5c, 0x54, 0x58, 0x5a, 
0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 
0x6d, 0x6d, 0x6d  
}; 
*/
//����Ϊ0247�İ�,����������
/*
u_char UnknowBufA[67] = {        // δ֪���͵İ�,��ͬ�İ汾�᲻ͬ
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00, 
0x00, 0x31, 0x02, 0x01, 0x00, 0x31, 0x02, 0x01,
0x16, 0x1c, 0x5e, 0x2b, 0x31, 0x55, 0x25, 0x24,  
0x6f, 0x6e, 0x1c, 0x4d, 0x76, 0x69, 0x45, 0x58, 
0x66, 0x97, 0x15, 0x54, 0xee, 0x02, 0x16, 0x3a,  
0x71, 0x38, 0x01, 0x0b, 0x3b, 0x7e, 0x3d, 0x26, 
0x7c, 0x7c, 0x17, 0x0b, 0x46, 0x08, 0x32, 0x32, 
0x08, 0x46, 0x0b  
}; 
*/

u_char UnknowBufA[67] = {        // δ֪���͵İ�,��ͬ�İ汾�᲻ͬ
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00, 
    0x00, 0x31, 0x02, 0x01, 0x00, 0x31, 0x02, 0x01,
    0x16,

    0x44,0x5F,0x28,0x33,0x57,0x26,0x25,0x37,0x36,0x1D,0x4E,0x74,0x6B,0x46,0x59,0x3E,0x62,0x8D,0xFE,0xA2,

    0x02, 0x16, 0x3a,  
    0x71, 0x38, 0x01, 0x0b, 0x3b, 0x7e, 0x3d, 0x26, 
    0x7c, 0x7c, 0x17, 0x0b, 0x46, 0x08, 0x32, 0x32, 
    0x08, 0x46, 0x0b 
}; 

u_char UnknowBufB[71] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00, 
    0x00, 0x31, 0x02, 0x01, 0x00, 0x31, 0x02, 0x01,
    0x16, 

    0x4E,0x09,0x2E,0x31,0x55,0x20,0x73,0x3D,0x3C,0x4B,0x48,0x76,0x69,0x40,0x0F,0x34,0xDD,0x6E,0x86,0xA7, 

    0x02, 0x16, 0x3a,
    0x71, 0x38, 0x01, 0x0b, 0x3b, 0x7e, 0x3d, 0x26,
    0x7c, 0x7c, 0x17, 0x0b, 0x46, 0x08, 0x32, 0x32,
    0x08,0x46,0x0b,0x00,0x00,0x00,0x00
};

u_char PasswordBuf[60]    = {        // ��������İ� 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00, 
    0x00, 0x1c, 0x02, 0x00, 0x00, 0x1c, 0x04, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
}; 

u_char SessionBuf[60] = {        // ά�ֶԻ��İ� 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00, 
    0x00, 0x15, 0x02, 0x00, 0x00, 0x15, 0x14, 0x00, 
    0x15, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
}; 

u_char SendUsernameBuf[60] = {    // �����û����İ� 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00, 
    0x00, 0x15, 0x02, 0x00, 0x00, 0x15, 0x01, 0x15,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00
}; 


void encode(char *strConst,unsigned char *strDest,int iSize)
{
    char *temp;

    temp = NULL;
    temp = malloc(iSize);
    assert(temp!=NULL); // check memory allocate fatal failure

    int iTimes = iSize/strlen(strConst);
    int i; 
    for( i=0;i<iTimes;i++)
        memcpy(temp+i*strlen(strConst),strConst,strlen(strConst));
    memcpy(temp+iTimes*strlen(strConst),strConst,iSize%strlen(strConst));


    for(i=0;i<iSize;i++)
        strDest[i] = strDest[i]^temp[i];
    for(i=0;i<iSize;i++)
        strDest[iSize-i-1] = strDest[iSize-i-1]^temp[i]; 
    free(temp);
}

unsigned long _rand(unsigned long t)
{
    t = t * 0x343fd;
    t = t + 0x269ec3;
    return (t>>0x10)&0x7fff;
    /*_asm
    {
    mov eax,t
    sar eax,0x10
    and eax,0x7fff
    }*/
}


void GetIPAddr();
void GetMacAddr();

bool SetMacAddress(const u_char *MacAddress);
bool SetUsername(const char *Username);
bool SetIpAddress(const u_char *IpAddress); 
void SetPassword(const char *Password);        // �������� 


void ChangeDestMac(const u_char *MAC);    // ���ĸ��ְ���Destination MAC 
bool OpenOneX(char* device);    // �����������豸���������豸 
void CloseOneX();            // �ر������豸 
void OneXLoop();        // ����������ѭ��(�ؼ�)

bool SetSessionID(const u_char *SessionID);    // ���öԻ�ά�ְ���ID 
void SetUsernameID(const u_char *UsernameID);    // ���÷����û�������ID 
void SetPasswordID(const u_char *PasswordID);    // ���÷����������ID 
void SetMd5Buf(const u_char *ID, const u_char *chap);    // �������ܺ�Ļظ�����

bool Connect();        // �����������ӵİ� 
bool Disconnect();    // ���ͶϿ����ӵİ�
//���ݰ����������� 
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data); 

//BOOL WINAPI ConsoleHandler(DWORD CEvent);
void config();
void readconfig();
void writeconfig();
void supplicant();


int main(int argc, char* argv[]) 
{ 
    printf("      This version is compitible with H3C's 802.1x Clent V2.20-0247\n");
    printf("      Coded by AGanNo2\n");
    printf("      Support site:http://AGanNo2.ys168.com\n");
    printf("      Email: AGanNo2@163.com\n");
    printf("      Thanks for thorqq!\n");
    printf("      To run it, you must be the root or you can change the privilege by command chmod or chown!\n");
    if(!(file=fopen("h3com802.1x.conf","r")))
    {
        config();
    }
    else
    {
        readconfig();
    }

    supplicant();

    //��ʼ��׽
    //pcap_loop(adhandle, 0, packet_handler, NULL); 	
    //�����������ݰ�
    //pcap_sendpacket(adhandle,DisconnectBuf,60);	
    return 0; 
} 

void config()
{
    printf("The config file h3com802.1x.conf doesn't exist,this may be the first you use it��Let's begin to config!\n");
    printf("any time you want to modify you account info,please fist delete the file h3com802.1x.conf!\n");
    printf("please input your username(max length 20):  ");
    scanf("%s",stUserData.username); // liuqun debug
    printf("please input your password(max length 20): ");
    scanf("%s",stUserData.password); // liuqun debug
    printf("%s\n",stUserData.username);
    printf("%s\n",stUserData.password);
    GetIPAddr();
    GetMacAddr();	
}


void readconfig()
{
    char line[100];
    printf("reading config info...\n");

    while(fgets(line,100,file))
    {
        line[strlen(line)-1]='\0';
        if(!memcmp("username",line,strlen("username")))
        {
            memcpy(stUserData.username,(line+sizeof("username")),sizeof(stUserData.username));
            printf("%s\n",stUserData.username);
            continue;
        }
        if(!memcmp("password",line,strlen("password")))
        {
            memcpy(stUserData.password,(line+sizeof("password")),sizeof(stUserData.password));
            printf("********\n");
            continue;
        }
        if(!memcmp("Ip",line,strlen("Ip")))
        {
            memcpy(stUserData.Ip,(unsigned char *)(line+sizeof("Ip")),sizeof(stUserData.Ip));
            sprintf(strIP,"%d.%d.%d.%d",stUserData.Ip[0],stUserData.Ip[1],stUserData.Ip[2],stUserData.Ip[3]);
            printf("%s\n",strIP);
            continue;
        }
        if(!memcmp("Mac",line,strlen("Mac")))
        {
            memcpy(stUserData.Mac,(unsigned char *)(line+sizeof("Mac")),sizeof(stUserData.Mac));
            printf("%02x-%02x-%02x-%02x-%02x-%02x\n",stUserData.Mac[0],stUserData.Mac[1],
                stUserData.Mac[2],stUserData.Mac[3],stUserData.Mac[4],stUserData.Mac[5]);
            continue;
        }
        if(!memcmp("nic",line,strlen("nic")))
        {
            memcpy(stUserData.nic,(line+sizeof("nic")),sizeof(stUserData.nic));
            printf("%s\n",stUserData.nic);
            break;
        }
    }
    fclose(file);
}


void writeconfig()
{		
    file=fopen("h3com802.1x.conf","w"); 
    fputs("username=",file);
    fputs(stUserData.username,file);
    fputs("\n",file);

    fputs("password=",file);
    fputs(stUserData.password,file);
    fputs("\n",file);

    fputs("Ip=",file);
    fputc(stUserData.Ip[0],file);
    fputc(stUserData.Ip[1],file);
    fputc(stUserData.Ip[2],file);
    fputc(stUserData.Ip[3],file);
    fputs("\n",file);

    fputs("Mac=",file);
    fputc(stUserData.Mac[0],file);
    fputc(stUserData.Mac[1],file);
    fputc(stUserData.Mac[2],file);
    fputc(stUserData.Mac[3],file);
    fputc(stUserData.Mac[4],file);
    fputc(stUserData.Mac[5],file);
    fputs("\n",file);

    fputs("nic=",file);
    fputs(stUserData.nic,file);
    fputs("\n",file);
    fclose(file);
}

void supplicant()
{
    //����֤��Ϣд�� ��
    SetMacAddress(stUserData.Mac);
    SetIpAddress(stUserData.Ip);
    SetUsername(stUserData.username);
    SetPassword(stUserData.password);

    if(!(OpenOneX(stUserData.nic)))	//������
    {
        printf("can't open net card!\n");
        exit(1);
    }
    Connect();	//�����������Ӱ�
    OneXLoop();		//����������ѭ��
}

//��windows���ݵ�GetTickCount����
long GetTickCount()
{
    struct tms tm;
    return times(&tm);

}

// �������ݰ��Ļص����� 
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data) 
{ 
    //ȷ��������������802.1x�İ�
    if(!memcmp(pkt_data+0x0c,PType,4)&&
        !memcmp(pkt_data+0x00,stUserData.Mac,6))
    {

        // ��õ�һ���������еõ��Ժ�������Ҫ�ظ���MAC��ַ 
        if (FirstPacket) 
        { 
            printf("first packet\n");
            FirstPacket = false; 
            ChangeDestMac(pkt_data + 0x06); 
        } 
        // �ж��Ƿ�Ϊ�Ի�ά�ְ���Ҫ�����û����İ�(�����ְ���������һ����) 
        if (!memcmp(pkt_data + 0x10, SessionFlagA, 3) &&  
            (!memcmp(pkt_data + 0x14, SessionFlagB, 3) ||
            !memcmp(pkt_data + 0x14, SessionFlagC, 3)) ) 
        { 
            // ������͹��û������ͶԻ�ά�ְ� 
            //if (UserSended) 
            if (online)//�Ѿ����ߣ����ͶԻ�ά�ְ�
            { 
                SetSessionID(pkt_data + 0x13); 
                pcap_sendpacket(fp, SessionBuf, 60);
            } 
            else    // ����ͷ����û��� 
            { 
                printf("user packet\n");
                SetUsernameID(pkt_data + 0x13);

                pcap_sendpacket(fp, UnknowBufB, 67);

                pcap_sendpacket(fp, SendUsernameBuf, 60); 
            } 
            return; 
        }

        // �ж��Ƿ�Ϊδ֪���͵İ� 
        if (!memcmp(pkt_data + 0x10, SessionFlagA, 3) && 
            !memcmp(pkt_data + 0x14, UnknowFlagA, 3)) 
        { 

            /*if( bVersionAFirstRequest == true )
            {
            bVersionAFirstRequest = false;
            return;
            }
            */
            printf("unknown packet\n");
            // ���Ͷ�Ӧ�Ļظ���(������������ǹ̶���,�汾Э��) 

            unsigned long uRand = _rand((unsigned long)time(NULL));
            unsigned long magic = uRand * GetTickCount();

            //unsigned long magic = 0x392df6ea;

            //��ʼ��strMagic,��magic��16������ʽת��Ϊ�ַ����������
            char strMagic[9]={0};
            unsigned char strTemp[4] = {0};
            memcpy(strTemp,(unsigned char *)&magic,4);
            sprintf(strMagic,"%02x%02x%02x%02x",strTemp[0],strTemp[1],strTemp[2],strTemp[3]);
            //printf("%s\n",strMagic);
            //��ʼ���汾��Ϣ
            unsigned char version[20];
            memset(version,0,sizeof(version));
            memcpy(version,"CH V2.20-0247",strlen("CH V2.20-0247"));
            memcpy(version+16,(unsigned char *)&magic,4);

            //encode("eaf62d39",version,0x10);
            encode(strMagic,version,0x10);

            encode("HuaWei3COM1X",version,0x14);
            memcpy(UnknowBufA+25,version,20);
            pcap_sendpacket(fp, UnknowBufA, 67);
            //pcap_sendpacket(fp, UnknowBufB, 67);
            return; 
        }

        // �ж��Ƿ�ΪҪ��������İ� 
        if (!memcmp(pkt_data + 0x10, RequestPwdFlagA, 3) && 
            !memcmp(pkt_data + 0x14, RequestPwdFlagB, 3)) 
        { 
            printf("passwd packet\n");
            SetPasswordID(pkt_data + 0x13); 
            SetMd5Buf(pkt_data + 0x13, pkt_data + 0x18);
            pcap_sendpacket(fp, PasswordBuf, 60); 
            return; 
        }

        // �ж��Ƿ�Ϊ��֤�ɹ��İ� code=3
        if (!memcmp(pkt_data + 0x10, SuccessFlagA, 3) &&  
            !memcmp(pkt_data + 0x14, SuccessFlagB, 3)) 
        { 
            printf("success\n");
            if (online == false) 
            { 
                online = true; 
            } 
            writeconfig();
            return; 
        } 

        // failure�� code=4
        if (!memcmp(pkt_data + 0x10, ByeFlagA, 3))
        {
            // �ж��Ƿ�Ϊ���߳ɹ��İ� 
            if(!memcmp(pkt_data + 0x14, ByeFlagB, 3)) 
            { 
                if (online == true) 
                { 
                    online = false; 
                    printf("Logging off successful!\n");
                } 
            }
            else// �������߰�,���������ԭ��
            {
                printf("%s\n",(pkt_data+0x18));
                config();
                supplicant();
            }
            system("pause");
            exit(1);
            return;
        }



        //��ʾ��֤�ɹ���Ϣ
        if(*(pkt_data +0x12) == 0x0a && *(pkt_data +0x1a) == 0xc4
            && *(pkt_data +0x1b) == 0xfa)
        {
            u_char a[0x100] = {0};
            memcpy(a, pkt_data+0x1a, *(pkt_data +0x11)-4);
            a[*(pkt_data +0x11)-3] = '\0';
            for(int i= 0; i<0x100; i++)
            {
                if((a[0x100-i] == 0x34) && (a[0x100-i+1] == 0x86))
                {
                    a[0x100-i] = '\n';
                    a[0x100-i+1] = '\n';
                    break;
                }
            }
            printf("%s\n", a);
        }		
    }
}


void GetIPAddr()
{
    pcap_if_t *alldevs; 
    pcap_if_t *d; 
    int inum; 
    int i=0; 
    pcap_t *adhandle; 
    char errbuf[PCAP_ERRBUF_SIZE]; 
    pcap_addr_t *a;
    unsigned long uIP;
    //��ȡ�豸�б� 
    if (pcap_findalldevs(&alldevs, errbuf) == -1) 
    { 
        fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf); 
        exit(1); 
    } 

    //�����б�
    for(d=alldevs; d; d=d->next) 
    { 
        //printf("%d. %s", ++i, d->name);
        printf("%d.",++i);
        if (d->name) 
            printf(" (%s)\n", d->name); 
        else 
            printf(" (No name available)\n");

    } 

    if(i==0) 
    { 
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n"); 
        exit(1); 
    } 

    printf("\nPlease choose net card (1-%d):",i); 
    scanf("%d", &inum); 

    if(inum < 1 || inum > i) 
    { 
        printf("\nInterface number out of range.\n"); 
        //�ͷ��豸�б� 
        pcap_freealldevs(alldevs); 
        exit(1); 
    } 

    //ת��ѡ����豸 
    for(d=alldevs, i=0; i< inum-1;d=d->next, i++); 

    //memcpy(stUserData.nic,d->name,strlen(d->name));
    //printf("%s\n",stUserData.nic);


    for(a=d->addresses;a;a=a->next)
    {
        if(a->addr && (a->addr->sa_family == AF_INET) )
        {
            uIP = ((struct sockaddr_in *)a->addr)->sin_addr.s_addr;
            memcpy( stUserData.Ip,(unsigned char *)&uIP,4);
            memcpy( stUserData.nic,d->name,strlen(d->name)+1);
            break;
        }
    }

    //ȡ��IP��ַ
    //a=d->addresses;
    //nsigned long ulIP=((struct sockaddr_in *)a->addr)->sin_addr.s_addr;
    //memcpy(stUserData.Ip,&ulIP,4);
    printf("%s\n",stUserData.nic);
    sprintf(strIP,"%d.%d.%d.%d",stUserData.Ip[0],stUserData.Ip[1],stUserData.Ip[2],stUserData.Ip[3]);
    //printf("\nlistening on %s...\n", d->description);
    printf("%s\n",strIP);
    //�����Ѿ�����Ҫ�豸�б���, �ͷ���
    pcap_freealldevs(alldevs);
}

void GetMacAddr()
{
    //����Ϊ�õ�MAC��ַ�Ĵ���
    //Linux�µķ���
    libnet_t  *l=NULL;
    char  l_errbuf[LIBNET_ERRBUF_SIZE];
    struct libnet_ether_addr *l_ether_addr;
    if((l=libnet_init(LIBNET_LINK, stUserData.nic,l_errbuf))==NULL)
    {
        printf("libnet_init: %s\n", l_errbuf);
    }
    if((l_ether_addr=libnet_get_hwaddr(l))==NULL)
    { 
        printf("unable to get local mac address :%s\n",libnet_geterror(l));
        exit(1);
    }
    memcpy(stUserData.Mac,l_ether_addr,sizeof(stUserData.Mac));

}

void ChangeDestMac(const u_char *MAC)    // ���ĸ��ְ���Destination MAC 
{ 
    memcpy(DisconnectBuf, MAC, 6); 
    memcpy(UnknowBufA, MAC, 6); 
    memcpy(UnknowBufB, MAC, 6);
    memcpy(PasswordBuf, MAC, 6); 
    memcpy(SessionBuf, MAC, 6); 
    memcpy(SendUsernameBuf, MAC, 6); 
} 

bool SetSessionID(const u_char *SessionID)    // ���öԻ�ά�ְ���ID 
{ 
    if (SessionBuf == NULL) 
        return false; 

    memcpy(SessionBuf + 0x13, SessionID, 1); 

    return true; 
} 

void SetUsernameID(const u_char *UsernameID)    // ���÷����û�������ID 
{     
    memcpy(SendUsernameBuf + 0x13, UsernameID, 1); 
} 
void SetPasswordID(const u_char *PasswordID)    // ���÷����������ID 
{ 
    memcpy(PasswordBuf + 0x13, PasswordID, 1); 
} 

void SetMd5Buf(const u_char *ID, const u_char *chap)    // �������ܺ�Ļظ����� 
{ 
    u_char TmpBuf[1 + 64 + 16]; 
    u_char digest[16]; 

    memcpy(TmpBuf + 0x00, ID, 1);
    memcpy(TmpBuf + 0x01, Passwd, PasswdLen); 
    memcpy(TmpBuf + 0x01 + PasswdLen, chap, 16); 

    (void) MD5(TmpBuf, (1+PasswdLen+16), digest); // ����openssl�ṩ��MD5���� (LiuQun 2013-1-26)
    memcpy(PasswordBuf + 0x18, digest, 16); 
} 

// ����Source Mac 
bool SetMacAddress(const u_char *MacAddress) 
{ 
    if (SessionBuf == NULL)
        return false; 

    memcpy(SessionBuf + 0x06, MacAddress, 6); 
    memcpy(SendUsernameBuf + 0x06, MacAddress, 6); 
    memcpy(ConnectBuf + 0x06, MacAddress, 6); 
    memcpy(DisconnectBuf + 0x06, MacAddress, 6); 
    memcpy(UnknowBufA + 0x06, MacAddress, 6);
    memcpy(UnknowBufB + 0x06, MacAddress, 6);
    memcpy(PasswordBuf + 0x06, MacAddress, 6); 

    return true; 
} 

bool SetIpAddress(const u_char *IpAddress) 
{ 
    if (SessionBuf == NULL) 
        return false; 

    memcpy(SessionBuf + 0x1a, IpAddress, 4); 
    memcpy(SendUsernameBuf + 0x19, IpAddress, 4); 

    return true; 
} 

bool SetUsername(const char *Username) 
{ 
    if (SessionBuf == NULL) 
        return false; 

    memcpy(SessionBuf + 0x1e, Username, strlen(Username)); 
    memcpy(SendUsernameBuf + 0x1d, Username, strlen(Username)); 
    memcpy(PasswordBuf + 0x28, Username, strlen(Username));

    //���ó���ֵ
    SessionBuf[0x11] = strlen(Username) + 0x0b; 
    SendUsernameBuf[0x11] = strlen(Username) + 0x0b; 
    PasswordBuf[0x11] = strlen(Username) + 0x16;

    SessionBuf[0x15] = strlen(Username) + 0x0b; 
    SendUsernameBuf[0x15] = strlen(Username) + 0x0b; 
    PasswordBuf[0x15] = strlen(Username) + 0x16;

    return true; 
} 

void SetPassword(const char *Password)        // �������� 
{ 
    strcpy(Passwd, Password); 
    PasswdLen = strlen(Passwd); 
} 


bool Connect()        // �����������ӵİ� 
{ 
    if (fp == NULL) 
        return false;     
    printf("start supplicant\n");
    return !pcap_sendpacket(fp, ConnectBuf, 60); 
} 

bool Disconnect()    // ���ͶϿ����ӵİ� 
{ 
    if (fp == NULL) 
        return false; 

    return !pcap_sendpacket(fp, DisconnectBuf, 60); 
} 

bool OpenOneX(char* device)    // �����������豸���������豸 
{ 
    char errbuf[PCAP_ERRBUF_SIZE]; 

    if ((fp = pcap_open_live(device, 200, 0, 20, errbuf)) == NULL)     
        return false; 

    return true; 
} 

void CloseOneX()            // �ر������豸 
{         
    if (fp != NULL) 
    { 
        pcap_close(fp); 
    } 		
} 

void OneXLoop()        // ����������ѭ��(�ؼ�) 
{ 
    if (fp == NULL) 
        return; 

    pcap_loop(fp, 0, packet_handler, NULL); 
} 
