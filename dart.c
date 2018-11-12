#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mysql/mysql.h>
#include <math.h>

//MySQL connection.
MYSQL my_connection,*conn_ptr; 
const char *pHostName = "localhost";  
const char *pUserName = "root";
const char *pPassword = "toor";
const char *pDbName = "sensors";
const unsigned int iDbPort = 3306;

//mysql_query(&conn, "create table hcoh(value DOUBLE,
//ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ");
//insert  into hcoh(value,ts)values(0.012312,null);
//create table HOCH(id int(8) primary key not null auto_increment,ppb DOUBLE,ugm3 DOUBLE,ppb2mgm3 DOUBLE,ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP);
//CREATE DATABASE sensors CHARACTER SET utf8 COLLATE utf8_bin;
//GRANT ALL PRIVILEGES ON sensors.* TO 'root'@'localhost' IDENTIFIED BY '*****';
//mysql_query(&mysql, "create table tmptab(c1 int, c2 varchar(20), c3 varchar(20))");

int init_mysql()
{
    conn_ptr = mysql_init(&my_connection);
    if( conn_ptr == NULL ) 
        return -1;
    if( !mysql_real_connect(&my_connection, pHostName, pUserName, pPassword, pDbName, iDbPort, NULL, 0))
        return -2;
    //if( mysql_query("set names utf8") )
    //    return -3;

    return 0;
}


static void skeleton_daemon()
{
    pid_t pid;
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

}
void init_tty(int fd)
{
        struct termios newtio;
        struct termios oldtio;
        bzero(&newtio,sizeof(struct termios));
	tcgetattr(fd,&oldtio); 
	newtio.c_cflag |= CLOCAL | CREAD;
	cfsetispeed(&newtio,B9600); 
	cfsetospeed(&newtio,B9600);
	newtio.c_cflag &= ~CSIZE; 
	newtio.c_cflag |= CS8; 
	newtio.c_cflag &= ~PARENB; newtio.c_iflag &= ~INPCK; 
	newtio.c_cflag &= ~CSTOPB; newtio.c_cc[VTIME] = 0; 
	newtio.c_cc[VMIN] = 0; 
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
       	newtio.c_oflag &= ~OPOST; 
	tcflush(fd,TCIOFLUSH); 
	tcsetattr(fd,TCSANOW,&newtio); 
}
// this cmd set is for dart wx-s
//和问答式
unsigned char  SET_POST[] = {0xFF,0x01,0x78,0x41,0x00,0x00,0x00,0x00,0x46};
unsigned char POST_CMD[] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
//                0      1      2      3       4     5      6      7      8
//return value {0xFF,0x86,0x00,0x2A,0x00,0x00,0x00,0x00,0x79};
//               起始位 命令 气体浓度高位(ug/m3) 气体浓度低位(ug/m3) 保留 保留 气体浓度低高位(ppb) 气体浓度低位(ppb) 校验值
//气体浓度值=气体浓度高位*256+气体浓度低位
//(浓度度高位和浓度低位需从 16 进制换算为 10 进制后再代入本公式计算)


//主动上传
unsigned char SET_PUT[]  = {0xFF,0x01,0x78,0x40,0x00,0x00,0x00,0x00,0x47};
//0       1       2       3       4         5          6        7        8
//起始位 气体名称  单位   小数位数 气体浓度高位 气体浓度低位 满量程高位 满量程低位 校验值
//0x00 CH2O=0x17 0x00   0x25    0x00          0x07      0x07   0xD0     0x25      

unsigned char FucCheckSum(unsigned char *i, unsigned char ln)
{
	unsigned char j, tempq = 0;
	i += 1;
	for(j = 0; j<(ln-2); j++)
	{
		tempq += *i;
		i++;
	}
	tempq = (~tempq)+1;
	return(tempq);
}


int main() 
{
	
        //skeleton_daemon();
       	int fd = -1;
       	float ppb = 0.0,mgm3=0,ppb2mgm3=0;
        init_mysql();
       	unsigned char buf[10],chksum;
       	fd = open("/dev/ttyAMA0",O_RDWR); 
	if(fd < 0)
	{ 
	       	exit(1);
       	}
       	init_tty(fd); 
	char sql[100];
	memset(sql,0,100);
	//设置被动模式
	//write(fd,SET_POST,9);

        while(1)
	{
	       	bzero(buf,10);
		//发送接收数据指令
		if (write(fd,POST_CMD,9)<0)
		{
			usleep(1000);
			continue;
		}
		//usleep(100*1000);
	       	if(read(fd, buf, 9)>=9)
		{ 
			buf[10]='\0';
			chksum=FucCheckSum(buf,9);
			printf("%04x %04x %04x %04x %04x %04x %04x %04x %04x\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8]);
			if(buf[8] != chksum)
			{
				printf("buf[8]= %x,chksum = %x\n",buf[8],chksum);
				continue;
			}
                        //解析被动上传数据计算
			ppb = (buf[6]*256+buf[7]);
			ppb2mgm3 = (30.03/22.4)*ppb*0.001;
			mgm3 = (buf[2]*256+buf[3])*0.001;
			sprintf(sql,"INSERT INTO HOCH(ppb,mgm3,ppb2mgm3)VALUES(%f,%f,%f);",ppb,mgm3,ppb2mgm3);
			mysql_query(&my_connection, sql);
			memset(sql,0,100);
			sleep(60);
		}
	} 
       	close(fd);
    mysql_close(&my_connection);
}
