//编程实现利用广播星历计算卫星位置并与精密星历比较。
//编程要求：
//主函数输入：年 月 日 时 分 秒 卫星编号；
//输出：卫星坐标x y z

#include<iostream>
#include<afx.h>
using namespace std;
#define GM 3.986005e+014
#define we 0.00007292115
//基本广播星历块
struct EPHEMERISBLOCK
//每小时一个卫星对应一个基本星历块
{
	int PRN;//PRN号
	double a0,a1,a2;//时间改正数
	double IODE,Crs,Deltan,M0;// ORBIT - 1广播轨道1
	double Cuc,e,Cus,SqrtA;// ORBIT - 2广播轨道
	double Toe,Cic,OMEGA,Cis;// ORBIT - 3
	double i0,Crc,omega,OMEGAdot;// ORBIT - 4
	double IDOT,GpsWeekNumber,L2C,L2P;// ORBIT - 5
	double SatAccuracy,SatHealth,TGD,IODC;// ORBIT - 6
};
struct EPHEMERISBLOCK *m_pGpsEphemeris = NULL;
//GPS时
struct GPSTIME
{
	int weekno;
	double weekSecond;
};
struct GPSTIME gpstime;
int uPRN = 0;

int Calendar2GpsTime (int nYear, int nMounth, int nDay, int nHour, int nMinute, double dSecond, double &WeekSecond);
int BrodcastEphemeris(CString strEpheNam, int &EphemerisBlockNum, EPHEMERISBLOCK *m_pGpsEphemeris);


// 函数：根据广播星历文件计算卫星位置
int BrodcastEphemeris(CString strEpheNam, EPHEMERISBLOCK *m_pGpsEphemeris)
{
	int EphemerisBlockNum = 0;
	int HeadLineNum = 0;
	int WeekNo;
	double WeekSecond;
	//打开文件
	CStdioFile pfEph;
	BOOL IsEn = pfEph.Open(strEpheNam, CFile::modeRead);
	if(!IsEn) return 0;
	//读入头文件
	CString strLine;

	while(IsEn)
	{
		IsEn = pfEph.ReadString(strLine);
		HeadLineNum++;
		int index = strLine.Find(_T("END OF HEADER"));
		if( -1 != index )
			break;
	}
	//计算星历块数
	int AllNum = 0;
	while(IsEn)
	{
		pfEph.ReadString(strLine);
		strLine+="\0";
		int L = strLine.GetLength();
		if(L<10||L>127) IsEn = 0 ;
		else AllNum++;
	}
	//临时读入星历块
	EphemerisBlockNum = (AllNum + 1) / 8;
	m_pGpsEphemeris = new EPHEMERISBLOCK[EphemerisBlockNum];
	GPSTIME  *pGpsTime = new GPSTIME[EphemerisBlockNum];
	memset(m_pGpsEphemeris,0,sizeof(m_pGpsEphemeris));


	if(!m_pGpsEphemeris || !pGpsTime) return 0;
	//将文件指针调整到数据位置
	pfEph.SeekToBegin();
	for(int i=0; i<HeadLineNum; i++)
		IsEn = pfEph.ReadString(strLine);
    //定义读取的参数
	int mPrn;//卫星号PRNo 
	int year,month,day,hour,minute;//卫星钟参考时刻
	double   msecond; 
	double   a0,a1,a2;//卫星钟飘参数
	double   IODE,Crs,DeltN,M0;//数据星历发布时间，在轨道径向方向上周期改正正弦的振幅
	double   Cuc,e,Cus,sqrtA;//轨道延迹方向上周期改正余弦振幅 、扁心率、轨道延迹方向上周期改正正弦振幅 、长半轴平方根 
	double   Toe,Cic,OMEGA,Cis;//星历参考时刻、轨道倾角周期改正余弦项振幅、参考时刻升交点赤径主项、轨道倾角周期改正正弦项振幅
	double   i0,Crc,omega,OMEGADOT;//参考时间轨道倾角、在轨道径向方向上周期改正余余弦的振幅、近地点角距、升交点赤径在赤道平面中的长期变化
	double   IDOT,L2C,GPSWeek,L2P;////轨道倾角变化率、gps周
	double   AccuracyofSat,HealthofSat,TGD,IODC;//卫星精度、卫星健康、电离层延迟改正数


	for(int i=0; i<EphemerisBlockNum; i++)
	{
		//读取卫星PRN号，星历参考时间
		IsEn = pfEph.ReadString(strLine);
		strLine.Replace('D', 'e');
		sscanf(strLine,"%d %d %d %d %d %d %lf %lf %lf %lf",&mPrn,&year,&month,&day,&hour,&minute,&msecond, &a0, &a1, &a2);

		year += 2000;

		WeekNo = Calendar2GpsTime(year, month, day, hour, minute, msecond, WeekSecond);
		pGpsTime[i].weekno = WeekNo;
		pGpsTime[i].weekSecond = WeekSecond;

		IsEn = pfEph.ReadString(strLine);
		strLine.Replace('D', 'e');
		sscanf(strLine,"%lf %lf %lf %lf",&IODE, &Crs, &DeltN, &M0);

		//读 Cuc,e,Cus,sqrtA
		IsEn = pfEph.ReadString(strLine);
		strLine.Replace('D', 'e');
		sscanf(strLine,"%lf %lf %lf %lf",&Cuc, &e, &Cus, &sqrtA);

		//Toe,Cic,OMEGA,Cis;
		IsEn = pfEph.ReadString(strLine);
		strLine.Replace('D', 'e');
		sscanf(strLine,"%lf %lf %lf %lf",&Toe, &Cic, &OMEGA, &Cis);

		//i0,Crc,w,OMEGADOT
		IsEn = pfEph.ReadString(strLine);
		strLine.Replace('D', 'e');
		sscanf(strLine,"%lf %lf %lf %lf",&i0, &Crc, &omega, &OMEGADOT);

		//IDOT,L2Cod,GPSWeek,L2PCod
		IsEn = pfEph.ReadString(strLine);
		strLine.Replace('D', 'e');
		sscanf(strLine,"%lf %lf %lf %lf", &IDOT, &L2C, &GPSWeek, &L2P);

		//AccuracyofSat,HealthofSat,TGD,IODC
		IsEn = pfEph.ReadString(strLine);
		strLine.Replace('D', 'e');
		sscanf(strLine,"%lf %lf %lf %lf",&AccuracyofSat, &HealthofSat, &TGD, &IODC);
		//
		IsEn = pfEph.ReadString(strLine);
		//赋值
		m_pGpsEphemeris[i].PRN = mPrn;
		m_pGpsEphemeris[i].a0 = a0;
		m_pGpsEphemeris[i].a1 = a1;
		m_pGpsEphemeris[i].a2 = a2;

		//&IODE, &Crs, &DeltN, &M0
		m_pGpsEphemeris[i].IODE = IODE;
		m_pGpsEphemeris[i].Crs  = Crs;
		m_pGpsEphemeris[i].Deltan = DeltN;
		m_pGpsEphemeris[i].M0 = M0;
		//&Cuc, &e, &Cus, &sqrtA
		m_pGpsEphemeris[i].Cuc = Cuc;
		m_pGpsEphemeris[i].e = e;
		m_pGpsEphemeris[i].Cus = Cus;
		m_pGpsEphemeris[i].SqrtA = sqrtA;
		//Toe,Cic,OMEGA,Cis;
		m_pGpsEphemeris[i].Toe = Toe;
		m_pGpsEphemeris[i].Cic = Cic;
		m_pGpsEphemeris[i].OMEGA = OMEGA;
		m_pGpsEphemeris[i].Cis = Cis;
		//i0,Crc,omega,OMEGADOT
		m_pGpsEphemeris[i].i0 = i0;
		m_pGpsEphemeris[i].Crc = Crc;
		m_pGpsEphemeris[i].omega = omega;
		m_pGpsEphemeris[i].OMEGAdot = OMEGADOT;
		//iDOT,L2Cod,GPSWeek,L2PCod
		m_pGpsEphemeris[i].IDOT = IDOT;
		m_pGpsEphemeris[i].L2C = L2C;
		m_pGpsEphemeris[i].L2P = L2P;
		m_pGpsEphemeris[i].GpsWeekNumber = GPSWeek;
		//AccuracyofSat,HealthofSat,TGD,IODC
		m_pGpsEphemeris[i].SatAccuracy = AccuracyofSat;
		m_pGpsEphemeris[i].SatHealth = HealthofSat;
		m_pGpsEphemeris[i].TGD = TGD;
		m_pGpsEphemeris[i].IODC = IODC;

		//计算
		if(pGpsTime[i].weekno == gpstime.weekno && pGpsTime[i].weekSecond == gpstime.weekSecond && m_pGpsEphemeris[i].PRN == uPRN){
		double n0 = sqrt(GM)/pow(m_pGpsEphemeris[i].SqrtA,3); // 参考时刻平均角速度
		double n = n0 + m_pGpsEphemeris[i].Deltan; // 观测时刻平均角速度
		double M = m_pGpsEphemeris[i].M0 + n*(gpstime.weekSecond - m_pGpsEphemeris[i].Toe);// 计算观测瞬间的平近点角M
		double E0 = 0;
		double E1 = M + m_pGpsEphemeris[i].e*sin(E0);
		while(abs(E1- E0) > pow(0.000001,2)){ E0 = E1;  E1 = M + m_pGpsEphemeris[i].e*sin(E0);}// 计算偏近点角
		double f = atan((sqrt(1-m_pGpsEphemeris[i].e*m_pGpsEphemeris[i].e)*sin(E1)/(cos(E1)-m_pGpsEphemeris[i].e)));// 计算真近点角
		double u_ = m_pGpsEphemeris[i].omega + f;// 计算升交角距（未经改正的）
        double r_ = pow(m_pGpsEphemeris[i].SqrtA,2)*(1-m_pGpsEphemeris[i].e*cos(E1));//计算卫星向径（未经改正的）
		// 计算摄动改正项
		double gu = m_pGpsEphemeris[i].Cuc*cos(2*u_)+m_pGpsEphemeris[i].Cus*sin(2*u_);
		double gr = m_pGpsEphemeris[i].Crc*cos(2*u_)+m_pGpsEphemeris[i].Crs*sin(2*u_);
		double gi = m_pGpsEphemeris[i].Cic*cos(2*u_)+m_pGpsEphemeris[i].Cis*sin(2*u_);
		// 进行摄动改正
		double u = u_ + gu;
		double r = r_ + gr;
		double ii = m_pGpsEphemeris[i].i0 + gi + m_pGpsEphemeris[i].IDOT*(gpstime.weekSecond - m_pGpsEphemeris[i].Toe);
		// 计算卫星在轨道平面坐标系中的位置
		double x = r*cos(u);
		double y = r*sin(u);
		// 计算观测瞬间升交点经度L
		double L = m_pGpsEphemeris[i].OMEGA + m_pGpsEphemeris[i].OMEGAdot*(gpstime.weekSecond - m_pGpsEphemeris[i].Toe) - we*gpstime.weekSecond;
	    // 计算在瞬时地球坐标系中的坐标
		double X = x*cos(L) - y*cos(ii)*sin(L);
		double Y = x*sin(L) + y*cos(ii)*cos(L);
		double Z = y*sin(ii);
		printf("PRN:%d X=%f, Y=%f, Z=%f\n",m_pGpsEphemeris[i].PRN,X,Y,Z);}
	}
	pfEph.Close();
	if(pGpsTime) {delete []pGpsTime; pGpsTime = NULL; }
	return EphemerisBlockNum;
}

// 函数：日期换算成GPS时
int Calendar2GpsTime (int nYear, int nMounth, int nDay, int nHour, int nMinute, double dSecond, double &WeekSecond)
{
	int DayofMonth = 0;
	int DayofYear = 0;
	int weekno = 0;
	int dayofweek;
	int m;
	if (nYear < 1980 || nMounth < 1 || nMounth > 12 || nDay < 1 || nDay > 31)  return -1;
	//计算从1980年到当前的前一年的天数
	for( m = 1980 ; m < nYear ; m++ )
	{
		if ( (m%4 == 0 && m%100 != 0) || (m%400 == 0) ) 
		{
			DayofYear += 366;
		}
		else
			DayofYear += 365;
	}
	//计算当前一年内从元月到当前前一月的天数
	for( m = 1;m < nMounth; m++)
	{
		if(m==1 || m==3 || m==5 || m==7 || m==8 || m==10 || m==12)
			DayofMonth += 31;
		else if (m==4 || m==6 || m==9 || m==11) 
			DayofMonth += 30;
		else if (m ==2)
		{
			if ( (nYear%4 == 0 && nYear%100 != 0) || (nYear%400 == 0) )
				DayofMonth += 29;
			else 
				DayofMonth += 28;
				
		}
	}
	DayofMonth = DayofMonth + nDay - 6;//加上当月天数/减去1980年元月的6日		
	weekno = (DayofYear + DayofMonth) / 7;//计算GPS周
	dayofweek = (DayofYear + DayofMonth) % 7;
	//计算GPS 周秒时间
	WeekSecond = dayofweek*86400 + nHour*3600 + nMinute*60 + dSecond;
	
	return weekno;
}


int main(void)
{
//主函数输入：年 月 日 时 分 秒 卫星编号；
//输出：卫星坐标x y z
  CString strEpheNam;
  printf("输入文件名（如：D：15372370.txt）\n");
  scanf("%s",strEpheNam);
  int Year,Mounth,Day,Hour,Minute;double Second;
  Year = Mounth = Day = Hour = Minute = Second = 0;
  printf("\n输入时间及卫星编号（如：2006  8 25  6  0  0.0 16）：");// 如：2013 10 16  4  0  0.0 7
  scanf("%d", &Year);
  scanf("%d", &Mounth);
  scanf("%d", &Day);
  scanf("%d", &Hour);
  scanf("%d", &Minute);
  scanf("%f", &Second);
  scanf("%d", &uPRN);
  gpstime.weekno = Calendar2GpsTime (Year,Mounth,Day,Hour,Minute,Second, gpstime.weekSecond);
  printf("GPS时为：%d %f\n", gpstime.weekno, gpstime.weekSecond);
  BrodcastEphemeris(strEpheNam, m_pGpsEphemeris);
  return 1;
}
