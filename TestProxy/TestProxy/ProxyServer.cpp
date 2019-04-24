// Server.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "ProxyServer.h"
#include "stdafx.h"
#include "afxsock.h"
#include "Proxy_Parse.h"
#include <thread>
//#include<string>
//#include<WinSock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object

CWinApp theApp;

using namespace std;



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
		char request[5000] = { 0 }, dname[1000], ip[16], body_res[5000] = { 0 }, header_res[1000] = { 0 };
		while(1)
		{
			CSocket Connector;
			if (ProxySocket.Accept(Connector))
			{

				cout << "Da co Client ket Noi !!!" << endl << endl;

				memset(request, 0, sizeof request);
				memset(dname, 0, sizeof dname);
				memset(ip, 0, sizeof ip);

				int revd = 0;
				if ((revd = Connector.Receive(request, 5000, 0)) > 0)
				{
					cout << "nhan request thanh cong " << endl;
					cout << "tong do dai byte: " << revd << endl;
					cout << request << "\n\n";
				}
				//
				if (!IsGETMethod(request))
					continue;
				if (IsHTTPs(request))
				{
					cout << "is HTTPS" << endl;
					continue;
				}
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
				while (endhead <4)
				{
					Proxy_Server.Receive(header_res + id, 1, 0);
					//head.push_back(header_res[id]);
					cout << header_res[id];
					if (header_res[id] == '\r' || header_res[id] == '\n')
						endhead++;
					else endhead = 0;
					id++;
				}
				Connector.Send(header_res, id, 0);
				cout << "Xong header roi nhe !" << endl << endl;

				id = 0;
				while ((revd = Proxy_Server.Receive(body_res, 5000, 0)) > 0)
				{
					Connector.Send(body_res, revd, 0);
					cout << "Tong byte nhan lan thu " << id << " : " << revd << endl;
					//if (id == 0)
					//cout << body_res << endl;
					id++;
					memset(body_res, 0, sizeof body_res);
				}
				// Đóng kết nối tới server web:
				Proxy_Server.Close();
			}
			Connector.Close();
		}
		ProxySocket.Close();
	}

	return nRetCode;
}

