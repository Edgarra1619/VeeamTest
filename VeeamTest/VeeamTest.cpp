//
// VeeamTest.cpp : Defines the entry point for the application.
//

#include "VeeamTest.h"


using namespace std;
const filesystem::copy_options copyOptions = filesystem::copy_options::overwrite_existing;

struct logInfo {
	enum actionType{
		noChange = 0,
		addFile = 1,
		updateFile = 2,
		deleteFile = 3
	} logType;
	filesystem::path relativeFilePath;
	time_t changeTime = 0;
	logInfo(actionType type, filesystem::path relativePath) {
		logType = type;
		relativeFilePath = relativePath;

	}
	logInfo() {
		logType = noChange;
	}
};



//returns true if they are different
bool diffFiles(filesystem::path aFilePath, filesystem::path bFilePath) {
	if (filesystem::last_write_time(aFilePath) != filesystem::last_write_time(bFilePath)) {

		return true;
	}
	if (filesystem::file_size(aFilePath)!=filesystem::file_size(bFilePath)) {

		return true;
	}
	
	ifstream aFileStream(aFilePath.c_str());

	ifstream bFileStream(bFilePath.c_str());

	char testA, testB;
	//scans through the files, seems to work for now, needs binary testing
	while (aFileStream.get(testA)&&bFileStream.get(testB)) {
		if (testA != testB) {
			
			return true;
		}

	}
	return false;
}







logInfo checkFile(filesystem::path sourceDir, filesystem::path replicaDir, filesystem::path toCheck) {
	if (!filesystem::exists(sourceDir) && !filesystem::exists(replicaDir))
		return logInfo();
	filesystem::path sourceTest = sourceDir;
	sourceTest /= toCheck;
	filesystem::path replicaTest = replicaDir;
	replicaTest /= toCheck;
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

	if (filesystem::is_directory(sourceTest) || filesystem::is_directory(replicaTest)) {
		return logInfoR;
	}
	

	if (diffFiles(sourceTest, replicaTest)) {
		logInfoR.logType = logInfo::updateFile;
		return logInfoR;
	}


	return logInfoR;
}


logInfo makeChanges(filesystem::path sourceDir, filesystem::path replicaDir, logInfo toChange) {
	switch (toChange.logType) {
	case logInfo::noChange:
		break;
	case logInfo::deleteFile:
		filesystem::remove_all(replicaDir.append(toChange.relativeFilePath.u8string()));
		break;
	default:
		filesystem::copy(sourceDir.append(toChange.relativeFilePath.u8string()), replicaDir.append(toChange.relativeFilePath.u8string()), copyOptions);
		break;
	}
	toChange.changeTime = time(0);
	return toChange;
}

void writeChanges(const logInfo toWrite, ofstream* log) {
	if (!log) {
		return;
	}
	string logString;
	switch (toWrite.logType) {
	case logInfo::noChange:
		return;
		break;
	case logInfo::addFile:
		logString += "Added ";
		break;
	case logInfo::updateFile:
		logString += "Changed ";
		break;
	case logInfo::deleteFile:
		logString += "Deleted ";
		break;
	}
	logString += "file " + filesystem::absolute(toWrite.relativeFilePath).string() + "\n\n";
	logString = ctime(&toWrite.changeTime) + logString;
	*log << logString;
}



void checkDirectoryRecursive(filesystem::path sourcePath, filesystem::path replicaPath, ofstream* logFile) {
	

	for (filesystem::directory_entry const& dir_entry : filesystem::directory_iterator{ replicaPath }) {
		filesystem::path checkPath = filesystem::relative(dir_entry, replicaPath);
		logInfo log = checkFile(sourcePath, replicaPath, checkPath);
		if (dir_entry.is_directory()&&log.logType == log.noChange) {
			checkDirectoryRecursive(sourcePath.append(checkPath.string()), replicaPath.append(checkPath.string()), logFile);
		}
		
		writeChanges(makeChanges(sourcePath, replicaPath, log), logFile);
	}
}





void synchronizeLoop(filesystem::path sourceDir, filesystem::path replicaDir, int interval, ofstream *logFile) {
	
	while (true) {
		chrono::time_point nextSync = chrono::system_clock::now();
		nextSync += chrono::seconds(interval);
		//check for file deletions and changes
		checkDirectoryRecursive(sourceDir, replicaDir, logFile);

		//check for new files
		for (filesystem::directory_entry const& dir_entry : filesystem::recursive_directory_iterator{ sourceDir }) {
			writeChanges(makeChanges(sourceDir, replicaDir, checkFile(sourceDir, replicaDir, filesystem::relative(dir_entry, sourceDir))), logFile);
		}
		
		logFile->flush();
		this_thread::sleep_until(nextSync);


	}
}






int main(int argc, char* argv[])
{ 
	if (argc != 5) { 
		cout << "4 arguments needed, " << argc - 1 << " provided\n Args: [SourcePath][ReplicaPath][IntervalInSeconds][LogPath]" << endl;

		return -1;
	}
	filesystem::path sourcePath = argv[1];
	filesystem::path replicaPath = argv[2];
	int interval = stoi(argv[3]);
	ofstream log;
	try {
		if (!filesystem::exists(sourcePath)) filesystem::create_directory(sourcePath);
		if (!filesystem::exists(replicaPath)) filesystem::create_directory(replicaPath);
		log.open(argv[4], ios_base::app);
	}
	catch (exception exc) {
		cout << "File paths not accepted" << endl;
		return -1;
	}
	

	synchronizeLoop(sourcePath, replicaPath, interval, &log);


	log.close();
	//cout << filesystem::canonical(replicaPath)<<endl;
	return 0;
}
