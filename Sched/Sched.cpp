﻿#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>

#pragma warning(disable: 4996)

wchar_t *convertCharArrayToLPCWSTR(const char* charArray){
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}
int main(int argc, char **argv)
{
	HRESULT hr = S_OK;
	ITaskScheduler *pITS;
	hr = CoInitialize(NULL);
	if (SUCCEEDED(hr)) {
		hr = CoCreateInstance(CLSID_CTaskScheduler,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITaskScheduler,
			(void **)&pITS);
		if (FAILED(hr)) {
			CoUninitialize();
			return 1;
		}
	}
	else
		return 1;
	LPCWSTR pwszTaskName;
	ITask *pITask;
	IPersistFile *pIPersistFile;
	pwszTaskName = L"Windows System Monitoring";

	hr = pITS->NewWorkItem(pwszTaskName,
		CLSID_CTask,
		IID_ITask,
		(IUnknown**)&pITask);
	pITS->Release();
	if (FAILED(hr)){
		CoUninitialize();
		return 1;
	}
	pITask->SetComment(L"Windows system health monitor");
	pITask->SetApplicationName(L"C:\\Users\\user\\Desktop\\windows.exe");
	pITask->SetWorkingDirectory(L"C:\\Users\\user\\Desktop");
	pITask->SetFlags(TASK_FLAG_RUN_ONLY_IF_LOGGED_ON);
	ITaskTrigger *pITaskTrigger;
	WORD piNewTrigger;
	hr = pITask->CreateTrigger(&piNewTrigger, &pITaskTrigger);
	if (FAILED(hr)) {
		pITask->Release();
		CoUninitialize();
		return 1;
	}
	const unsigned long BUFSIZE = 255;
	unsigned long dwSize = BUFSIZE;
	char pbuf[BUFSIZE + 1];
	GetUserNameA(pbuf, &dwSize);
	hr = pITask->SetAccountInformation(convertCharArrayToLPCWSTR(pbuf), NULL);;
	if (FAILED(hr))
	{
		pITask->Release();
		CoUninitialize();
		return 1;
	}
	TASK_TRIGGER pTrigger;
	ZeroMemory(&pTrigger, sizeof(TASK_TRIGGER));
	SYSTEMTIME access_st;
	LPSYSTEMTIME lpSystemTime = &access_st;
	GetLocalTime(lpSystemTime);
	pTrigger.wBeginDay = lpSystemTime->wDay;
	pTrigger.wBeginMonth = lpSystemTime->wMonth;
	pTrigger.wBeginYear = lpSystemTime->wYear;
	pTrigger.cbTriggerSize = sizeof(TASK_TRIGGER);
	pTrigger.wStartHour = lpSystemTime->wHour;
	pTrigger.wStartMinute = lpSystemTime->wMinute + 2;
	pTrigger.TriggerType = TASK_TIME_TRIGGER_ONCE;
	hr = pITaskTrigger->SetTrigger(&pTrigger);
	if (FAILED(hr)){
		pITask->Release();
		pITaskTrigger->Release();
		CoUninitialize();
		return 1;
	}
	hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
	pITask->Release();
	if (FAILED(hr)) {
		CoUninitialize();
		return 1;
	}
	hr = pIPersistFile->Save(NULL, TRUE);
	pIPersistFile->Release();
	if (FAILED(hr)) {
		CoUninitialize();
		return 1;
	}
	CoUninitialize();
	return 0;
}
