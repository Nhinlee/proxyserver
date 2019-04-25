// Server.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "ProxyServer.h"
#include "stdafx.h"
#include <afxsock.h>
#include "Proxy_Parse.h"


//#include<string>
//#include<WinSock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object

CWinApp theApp;

using namespace std;

UINT Rev_Send(LPVOID Prams)
{
	cout << "Da co Client ket Noi !!!" << endl << endl;
	CSocket*Connector = (CSocket*)(Prams);
	char request[5000] = { 0 }, dname[1000], ip[16], body_res[5000] = { 0 }, header_res[1000] = { 0 };

	int revd = 0;
	if ((revd = Connector->Receive(request, 5000, 0)) > 0)
	{
		cout << "nhan request thanh cong " << endl;
		cout << "tong do dai byte: " << revd << endl;
		cout << request << "\n\n";
	}
	//
	if (!IsGETMethod(request))
	{
		Connector->Close();
		return 0;
	}
	/*if (IsHTTPs(request))
	{
		cout << "is HTTPS" << endl;
		continue;
	}*/
	//
	if (GetDomainName(request, dname) == false)
	{
		cout << "Get Domain Name False !!!";
	}
	//else cout << dname << endl;
	// GET IP:

	hostent *remoteHost = gethostbyname(dname);
	int i = 0;
	in_addr addr;
	while (remoteHost->h_addr_list[i] != 0) {
		addr.s_addr = *(u_long *)remoteHost->h_addr_list[i++];
		strcpy(ip, inet_ntoa(addr));
		printf("\tIPv4 Address #%d: %s\n", i, inet_ntoa(addr));
	}
	//cout << ip << endl;
	// Tao Socket proxy moi de ket noi toi trang web:
	CSocket Proxy_Server;
	Proxy_Server.Create();
	// Ket noi toi server:
	if (Proxy_Server.Connect(CA2W(ip), 80) != 0)
	{
		cout << "Ket noi toi Web thanh cong !!!" << endl;
	}
	revd = Proxy_Server.Send(request, revd, 0);

	// Nhan va gui ve lai cho client:
	//string head;
	int id = 0, endhead = 0;
	while (endhead < 4)
	{
		Proxy_Server.Receive(header_res + id, 1, 0);
		//head.push_back(header_res[id]);
		cout << header_res[id];
		if (header_res[id] == '\r' || header_res[id] == '\n')
			endhead++;
		else endhead = 0;
		id++;
	}
	Connector->Send(header_res, id, 0);
	cout << id << " Byte ---- Xong header roi nhe !" << endl << endl;

	id = 0;
	while ((revd = Proxy_Server.Receive(body_res, 5000, 0)) > 0)
	{
		Connector->Send(body_res, revd, 0);
		cout << "Tong byte nhan lan thu " << id << " : " << revd << endl;
		//if (id == 0)
		//cout << body_res << endl;
		id++;
		memset(body_res, 0, sizeof body_res);
	}
	// Đóng kết nối tới server web:
	cout << "Da nhan xong !" << endl;
	Proxy_Server.Close();
	//Đóng socket tới client:
	//Errors có thể ở chỗ Close vì bẩn chất là truyền con trỏ vô kiểu nên ko có phép Close(). Còn nếu close() ngoài hàm thì 
	// sẽ không detach Thread dc ạ.
	Connector->Close();
	//delete Connector;
	return 1;
}


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.

		// Khoi tao thu vien Socket
		if (AfxSocketInit() == FALSE)
		{
			cout << "Khong the khoi tao Socket Library";
			return FALSE;
		}

		CSocket ProxySocket; //cha
		
		// Tao socket cho server, dang ky port la 8888, giao thuc TCP
		if (ProxySocket.Create(8888, SOCK_STREAM, NULL) == 0) //SOCK_STREAM or SOCK_DGRAM.
		{
			cout << "Khoi tao that bai !!!" << endl;
			cout << ProxySocket.GetLastError();
			return FALSE;
		}
		else
		{
				cout << "Server khoi tao thanh cong !!!" << endl;
		}

		// Dung socket nay de listen cac request tu client:

		if (ProxySocket.Listen(5) == FALSE)
		{
			cout << "Khong the lang nghe tren port nay !!!" << endl;
			ProxySocket.Close();
			return FALSE;
		}
		
		while (1)
		{
		
			/*CSocket *Connector = new CSocket;*/
			CSocket Connector;
			if (ProxySocket.Accept(Connector))
			{
				AfxBeginThread(Rev_Send, (LPVOID)&Connector);//Nhờ thầy xem giùm em chỗ thread này với ạ.
				//Em chạy được nhưng nó báo lỗi khi đóng Connector Socket ạ.
			}
			else {
				Connector.Close();
			}
		}

		ProxySocket.Close();
	}
	//system("pause");
	return nRetCode;
}
