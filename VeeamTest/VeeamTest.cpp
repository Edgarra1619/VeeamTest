// VeeamTest.cpp : Defines the entry point for the application.
//

#include "VeeamTest.h"
#include <stdio.h>
#include <filesystem>
#include <thread>
#include <chrono>
#include <future>

using namespace std;

struct logInfo {
	enum actionType{
		noChange = 0,
		addFile = 1,
		updatedFile = 2,
		deleteFile = 3
	} logType;
	filesystem::path relativeFilePath;
	logInfo(actionType type, filesystem::path relativePath) {
		logType = type;
		relativeFilePath = relativePath;
	}
	logInfo() {
		logType = noChange;
	}
};

bool diffFiles(filesystem::path aFile, filesystem::path bFile) {

}



logInfo checkFile(filesystem::path sourceDir, filesystem::path replicaDir, filesystem::path toCheck) {
	filesystem::path sourceTest = sourceDir;
	sourceTest += toCheck;
	filesystem::path replicaTest = replicaDir;
	replicaTest += toCheck;
	logInfo logInfoR = logInfo();
	logInfoR.relativeFilePath = toCheck;

	
	


	
	if (filesystem::exists(sourceTest)&&!filesystem::exists(replicaTest) ){
		logInfoR.logType = logInfo::addFile;
		return logInfoR;

	}
	if (!filesystem::exists(sourceTest) && filesystem::exists(replicaTest) ) {
		logInfoR.logType = logInfo::deleteFile;
		return logInfoR;
	}

	return logInfoR;
}


logInfo makeChanges(filesystem::path sourceDir, filesystem::path replicaDir, logInfo toChange) {
	switch (toChange.logType) {
	case logInfo::noChange:
		break;
	case logInfo::deleteFile:
		break;
	default:
		filesystem::copy(sourceDir.concat(toChange.relativeFilePath.u8string()), replicaDir.concat(toChange.relativeFilePath.u8string()));
		break;
	}

	
	return toChange;
}

void writeChanges(const logInfo toWrite, FILE *log) {
	/*if (!log) return;
	switch (toWrite.actionType) {
	case logInfo::noChange:
		return;
	case logInfo::addFile:
		fprintf(log, "Added ");
		break;
	case logInfo::updatedFile:
		fprintf(log, "Changed ");
		break;
	case logInfo::deletedFile:
		fprintf(log, "Deleted ");
		break;
	}*/
	//fprintf(log, "file %ls\n", toWrite.relativeFilePath.c_str());
}


void synchronizeLoop(filesystem::path sourceDir, filesystem::path replicaDir, float interval, FILE *logFile) {
	

	//std::async(std::launch::async, [])
	
}






int main(int argc, char* argv[])
{
	if (argc != 3) { 
		cout << "2 arguments needed, " << argc - 1 << " provided" << endl;

		return -1;
	}
	filesystem::path sourcePath = argv[1];
	filesystem::path replicaPath = argv[2];


	try {
		if (!filesystem::exists(sourcePath)) filesystem::create_directory(sourcePath);
		if (!filesystem::exists(replicaPath)) filesystem::create_directory(replicaPath);
	}
	catch (exception exc) {
		cout << "File paths not accepted" << endl;
		return -1;
	}
	


	checkFile(sourcePath, replicaPath, "./test.txt");

	

	cout << filesystem::canonical(replicaPath)<<endl;
	return 0;
}
