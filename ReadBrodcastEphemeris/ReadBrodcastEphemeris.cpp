//���ʵ�����ù㲥������������λ�ò��뾫�������Ƚϡ�
//���Ҫ��
//���������룺�� �� �� ʱ �� �� ���Ǳ�ţ�
//�������������x y z

#include<iostream>
#include<afx.h>
using namespace std;
#define GM 3.986005e+014
#define we 0.00007292115
//�����㲥������
struct EPHEMERISBLOCK
//ÿСʱһ�����Ƕ�Ӧһ������������
{
	int PRN;//PRN��
	double a0,a1,a2;//ʱ�������
	double IODE,Crs,Deltan,M0;// ORBIT - 1�㲥���1
	double Cuc,e,Cus,SqrtA;// ORBIT - 2�㲥���
	double Toe,Cic,OMEGA,Cis;// ORBIT - 3
	double i0,Crc,omega,OMEGAdot;// ORBIT - 4
	double IDOT,GpsWeekNumber,L2C,L2P;// ORBIT - 5
	double SatAccuracy,SatHealth,TGD,IODC;// ORBIT - 6
};
struct EPHEMERISBLOCK *m_pGpsEphemeris = NULL;
//GPSʱ
struct GPSTIME
{
	int weekno;
	double weekSecond;
};
struct GPSTIME gpstime;
int uPRN = 0;

int Calendar2GpsTime (int nYear, int nMounth, int nDay, int nHour, int nMinute, double dSecond, double &WeekSecond);
int BrodcastEphemeris(CString strEpheNam, int &EphemerisBlockNum, EPHEMERISBLOCK *m_pGpsEphemeris);


// ���������ݹ㲥�����ļ���������λ��
int BrodcastEphemeris(CString strEpheNam, EPHEMERISBLOCK *m_pGpsEphemeris)
{
	int EphemerisBlockNum = 0;
	int HeadLineNum = 0;
	int WeekNo;
	double WeekSecond;
	//���ļ�
	CStdioFile pfEph;
	BOOL IsEn = pfEph.Open(strEpheNam, CFile::modeRead);
	if(!IsEn) return 0;
	//����ͷ�ļ�
	CString strLine;

	while(IsEn)
	{
		IsEn = pfEph.ReadString(strLine);
		HeadLineNum++;
		int index = strLine.Find(_T("END OF HEADER"));
		if( -1 != index )
			break;
	}
	//������������
	int AllNum = 0;
	while(IsEn)
	{
		pfEph.ReadString(strLine);
		strLine+="\0";
		int L = strLine.GetLength();
		if(L<10||L>127) IsEn = 0 ;
		else AllNum++;
	}
	//��ʱ����������
	EphemerisBlockNum = (AllNum + 1) / 8;
	m_pGpsEphemeris = new EPHEMERISBLOCK[EphemerisBlockNum];
	GPSTIME  *pGpsTime = new GPSTIME[EphemerisBlockNum];
	memset(m_pGpsEphemeris,0,sizeof(m_pGpsEphemeris));


	if(!m_pGpsEphemeris || !pGpsTime) return 0;
	//���ļ�ָ�����������λ��
	pfEph.SeekToBegin();
	for(int i=0; i<HeadLineNum; i++)
		IsEn = pfEph.ReadString(strLine);
    //�����ȡ�Ĳ���
	int mPrn;//���Ǻ�PRNo 
	int year,month,day,hour,minute;//�����Ӳο�ʱ��
	double   msecond; 
	double   a0,a1,a2;//������Ʈ����
	double   IODE,Crs,DeltN,M0;//������������ʱ�䣬�ڹ�������������ڸ������ҵ����
	double   Cuc,e,Cus,sqrtA;//����Ӽ����������ڸ���������� �������ʡ�����Ӽ����������ڸ���������� ��������ƽ���� 
	double   Toe,Cic,OMEGA,Cis;//�����ο�ʱ�̡����������ڸ���������������ο�ʱ��������ྶ������������ڸ������������
	double   i0,Crc,omega,OMEGADOT;//�ο�ʱ������ǡ��ڹ�������������ڸ��������ҵ���������ص�Ǿࡢ������ྶ�ڳ��ƽ���еĳ��ڱ仯
	double   IDOT,L2C,GPSWeek,L2P;////�����Ǳ仯�ʡ�gps��
	double   AccuracyofSat,HealthofSat,TGD,IODC;//���Ǿ��ȡ����ǽ�����������ӳٸ�����


	for(int i=0; i<EphemerisBlockNum; i++)
	{
		//��ȡ����PRN�ţ������ο�ʱ��
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

		//�� Cuc,e,Cus,sqrtA
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
		//��ֵ
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

		//����
		if(pGpsTime[i].weekno == gpstime.weekno && pGpsTime[i].weekSecond == gpstime.weekSecond && m_pGpsEphemeris[i].PRN == uPRN){
		double n0 = sqrt(GM)/pow(m_pGpsEphemeris[i].SqrtA,3); // �ο�ʱ��ƽ�����ٶ�
		double n = n0 + m_pGpsEphemeris[i].Deltan; // �۲�ʱ��ƽ�����ٶ�
		double M = m_pGpsEphemeris[i].M0 + n*(gpstime.weekSecond - m_pGpsEphemeris[i].Toe);// ����۲�˲���ƽ�����M
		double E0 = 0;
		double E1 = M + m_pGpsEphemeris[i].e*sin(E0);
		while(abs(E1- E0) > pow(0.000001,2)){ E0 = E1;  E1 = M + m_pGpsEphemeris[i].e*sin(E0);}// ����ƫ�����
		double f = atan((sqrt(1-m_pGpsEphemeris[i].e*m_pGpsEphemeris[i].e)*sin(E1)/(cos(E1)-m_pGpsEphemeris[i].e)));// ����������
		double u_ = m_pGpsEphemeris[i].omega + f;// ���������Ǿࣨδ�������ģ�
        double r_ = pow(m_pGpsEphemeris[i].SqrtA,2)*(1-m_pGpsEphemeris[i].e*cos(E1));//���������򾶣�δ�������ģ�
		// �����㶯������
		double gu = m_pGpsEphemeris[i].Cuc*cos(2*u_)+m_pGpsEphemeris[i].Cus*sin(2*u_);
		double gr = m_pGpsEphemeris[i].Crc*cos(2*u_)+m_pGpsEphemeris[i].Crs*sin(2*u_);
		double gi = m_pGpsEphemeris[i].Cic*cos(2*u_)+m_pGpsEphemeris[i].Cis*sin(2*u_);
		// �����㶯����
		double u = u_ + gu;
		double r = r_ + gr;
		double ii = m_pGpsEphemeris[i].i0 + gi + m_pGpsEphemeris[i].IDOT*(gpstime.weekSecond - m_pGpsEphemeris[i].Toe);
		// ���������ڹ��ƽ������ϵ�е�λ��
		double x = r*cos(u);
		double y = r*sin(u);
		// ����۲�˲�������㾭��L
		double L = m_pGpsEphemeris[i].OMEGA + m_pGpsEphemeris[i].OMEGAdot*(gpstime.weekSecond - m_pGpsEphemeris[i].Toe) - we*gpstime.weekSecond;
	    // ������˲ʱ��������ϵ�е�����
		double X = x*cos(L) - y*cos(ii)*sin(L);
		double Y = x*sin(L) + y*cos(ii)*cos(L);
		double Z = y*sin(ii);
		printf("PRN:%d X=%f, Y=%f, Z=%f\n",m_pGpsEphemeris[i].PRN,X,Y,Z);}
	}
	pfEph.Close();
	if(pGpsTime) {delete []pGpsTime; pGpsTime = NULL; }
	return EphemerisBlockNum;
}

// ���������ڻ����GPSʱ
int Calendar2GpsTime (int nYear, int nMounth, int nDay, int nHour, int nMinute, double dSecond, double &WeekSecond)
{
	int DayofMonth = 0;
	int DayofYear = 0;
	int weekno = 0;
	int dayofweek;
	int m;
	if (nYear < 1980 || nMounth < 1 || nMounth > 12 || nDay < 1 || nDay > 31)  return -1;
	//�����1980�굽��ǰ��ǰһ�������
	for( m = 1980 ; m < nYear ; m++ )
	{
		if ( (m%4 == 0 && m%100 != 0) || (m%400 == 0) ) 
		{
			DayofYear += 366;
		}
		else
			DayofYear += 365;
	}
	//���㵱ǰһ���ڴ�Ԫ�µ���ǰǰһ�µ�����
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
	DayofMonth = DayofMonth + nDay - 6;//���ϵ�������/��ȥ1980��Ԫ�µ�6��		
	weekno = (DayofYear + DayofMonth) / 7;//����GPS��
	dayofweek = (DayofYear + DayofMonth) % 7;
	//����GPS ����ʱ��
	WeekSecond = dayofweek*86400 + nHour*3600 + nMinute*60 + dSecond;
	
	return weekno;
}


int main(void)
{
//���������룺�� �� �� ʱ �� �� ���Ǳ�ţ�
//�������������x y z
  CString strEpheNam;
  printf("�����ļ������磺D��15372370.txt��\n");
  scanf("%s",strEpheNam);
  int Year,Mounth,Day,Hour,Minute;double Second;
  Year = Mounth = Day = Hour = Minute = Second = 0;
  printf("\n����ʱ�估���Ǳ�ţ��磺2006  8 25  6  0  0.0 16����");// �磺2013 10 16  4  0  0.0 7
  scanf("%d", &Year);
  scanf("%d", &Mounth);
  scanf("%d", &Day);
  scanf("%d", &Hour);
  scanf("%d", &Minute);
  scanf("%f", &Second);
  scanf("%d", &uPRN);
  gpstime.weekno = Calendar2GpsTime (Year,Mounth,Day,Hour,Minute,Second, gpstime.weekSecond);
  printf("GPSʱΪ��%d %f\n", gpstime.weekno, gpstime.weekSecond);
  BrodcastEphemeris(strEpheNam, m_pGpsEphemeris);
  return 1;
}
