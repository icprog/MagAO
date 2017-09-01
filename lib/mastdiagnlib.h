#ifndef MASTDIAGNLIB_H_INCLUDE
#define MASTDIAGNLIB_H_INCLUDE

#include "AOExcept.h"
#include "Logger.h"

#include <fstream>
#include <cstdlib>

using namespace std;

namespace Arcetri {

/********************************* INTERFACE **********************************/

/*
 * @Class: MastDiagnInterfaceException
 * Base class for all the Master Diagnostic interface exceptions.
 * @
 */
class MastDiagnInterfaceException: public AOException {
	public:
		MastDiagnInterfaceException(string message,
		                    		int errcode = MASTDIAGN_INTF_GENERIC_ERROR,
		                    		string file = "",
		                    		int line = 0): AOException(message, errcode, file, line) {}

};

/*
 * @Class: MastDiagnInterface
 * Interface class to send commands to Master Diagnostic
 * @
 */
class MastDiagnInterface {

	public:

		MastDiagnInterface(string masterdiagnosticidentity, int logLevel = Logger::LOG_LEV_INFO);

		virtual ~MastDiagnInterface();

		/*
		 * Dumps the diagnostic history to a file
		 * TODO returns the file name
		 */
		void dumpAdSecDiagnHistory() throw (MastDiagnInterfaceException);
		void dumpOptLoopDiagnHistory() throw (MastDiagnInterfaceException);

	private:

		void sendMessage(int msgCode) throw (MastDiagnInterfaceException);

		Logger* _logger;

		string _mastDiagnMsgDIdnty;

};


/********************************* LIBRARY **********************************/

/*
 * @Class: FramesHistory
 * A thread-safe cyclic buffer to store frames history
 * @
 */
 class FramesHistory {

	public:

		/*
		 * Creates an empty history
		 */
		FramesHistory(string historyName, unsigned int framesNumber, unsigned int framesSizeBytes) {

			_historyName = historyName;
			_frameSize = framesSizeBytes;
			_framesNumber = framesNumber;

			_allocateBuffer();

			pthread_mutex_init(&_mutex, NULL);
			Logger::get()->log(Logger::LOG_LEV_INFO, "FramesHistory: history %s (%d frames of %d Bytes) created", _historyName.c_str(), _framesNumber,  _frameSize);
		}


		 /*
		  * Destroy the history
		  */
		 ~FramesHistory() {
		 	delete[] _data;
		 }

		/*
		 * Insert a frame into the history in the next position
		 */
		void pushFrame(BYTE* frame) {
			lockBuffer();
			memcpy(_cursor, frame, _frameSize);
			Logger::get()->log(Logger::LOG_LEV_DEBUG, "FramesHistory::insertFrame: frame of size % Bytes inserted (relative pointer: %06x)", _frameSize, _cursor-_data);
			_cursor += _frameSize;
			if(_cursor == _dataEnd) {
				_cursor = _data;
			}
			unlockBuffer();
		}

		/*
		 * Get a frame from the history
		 */
		BYTE* getFrame(uint32 index) {
			if(index>=_framesNumber) {
				Logger::get()->log(Logger::LOG_LEV_WARNING, "FramesHistory::getFrame: index (%d) out-of-bound", index);
				return NULL;
			}
			else {
				Logger::get()->log(Logger::LOG_LEV_DEBUG, "FramesHistory::getFrame: frame retrieved");

				// ************* DEBUG ************* //
				if(Logger::get()->getLevel() >= Logger::LOG_LEV_TRACE) {
					BYTE* frame = _data+index*_frameSize;
					for(unsigned int i=0; i<_frameSize; i++) {
						printf("%03d ", (int)frame[i]);
					}
					printf("\n");
				}
				// ********************************** //

				return _data+index*_frameSize;
			}
		}

		/*
		 * Get the newer frame from the history
		 *
		 * NOTE
		 * Implemented in a more efficient way than calling
		 * getFrame(_framesNumber) method
		 */
		BYTE* getLastFrame() {

			// ************* DEBUG ************* //
			if(Logger::get()->getLevel() >= Logger::LOG_LEV_TRACE) {
				BYTE* frame =_cursor-_frameSize;
				for(unsigned int i=0; i<_frameSize; i++) {
					printf("%03d ", (int)frame[i]);
				}
				printf("\n");
			}
			// ********************************** //

			return _cursor-_frameSize;
		}

		string getHistoryName()  { return _historyName; }
		uint32 getFrameSize()    { return _frameSize; }
		uint32 getFramesNumber() { return _framesNumber; }

		/*
		 * Clear the history buffer , without deallocating memory
		 */
		void resetBuffer() {
			memset(_data, 0, _dataSize);
			_dataEnd = _data + _dataSize;
			_cursor = _data;
		}

	protected:

		friend class OFramesHistory;
		friend class IFramesHistory;

		/*
		 * History low-level access
		 */
		BYTE* getCursor()  { return _cursor; }
		BYTE* getData()    { return _data; }
		BYTE* getDataEnd() { return _dataEnd; }

		/*
		 * Concurrency management
		 */
		void lockBuffer() { pthread_mutex_lock(&_mutex); }
		void unlockBuffer() { pthread_mutex_unlock(&_mutex); }

		/*
		 * Insert a frame into the history in the specified position
		 */
		void insertFrame(BYTE* frame, uint32 pos) {
			if(pos >= _framesNumber) {
				Logger::get()->log(Logger::LOG_LEV_WARNING, "FramesHistory::insertFrame: frame position out-of-bounds");
				return;
			}
			else {
				lockBuffer();
				memcpy(_data+pos*_frameSize, frame, _frameSize);
				unlockBuffer();
				Logger::get()->log(Logger::LOG_LEV_DEBUG, "FramesHistory::insertFrame: frame inserted");
			}
		}

	private:

		/*
		 * Allocate the history buffer
		 */
		void _allocateBuffer() {
			_dataSize = _framesNumber*_frameSize;
			_data = new BYTE[_dataSize];
			resetBuffer();
		}


	protected:

		pthread_mutex_t _mutex;


	/*
	 * --- Implementation ---
	 * The buffer is logically divided in two dinamic parts:
	 *
	 * | Y1 | Y2 | Y3 | Y4 | ... | Ym | X1 | X2 | ... | Xn  |
	 *  ^_data                         ^_cursor              ^_dataEnd
	 *
	 * 	- X1 is the 'oldest' frame, the next replaced by insert(frame)
	 * 	- Xi are the frames belonging to the first part of the buffer
	 *  - Ym is the "newest" frame
	 *  - Yj are the frames belonging to the second part of the buffer
	 */
	private:

		string _historyName; // A name used to dump history to file

		uint32 _frameSize;   // Frame size (Bytes)
		uint32 _framesNumber; // Number of frames

		// BUFFER
		BYTE*  _data;		 // Pointer to the begin of data
		BYTE*  _dataEnd;	 // Pointer to the end of data used to have a curcular buffer
		uint32 _dataSize;	 // Total size of data (Bytes), equals to _dataEnd-_data
		BYTE*  _cursor;		 // Pointer to the next free "frame slot"
} ;

/*
 * @Class: OFramesHistory
 * A FramesHistory able to dump its content to a file
 * @
 */
class OFramesHistory {

	public:

		/*
		 * Creates an empty buffer ready to be filled
		 */
		OFramesHistory(string historyName, unsigned int framesNumber, unsigned int framesSizeBytes) {
			 _framesHistory = new FramesHistory(historyName, framesNumber, framesSizeBytes);
		}


		~OFramesHistory() {
			delete _framesHistory;
		}

		string getHistoryName()  { return _framesHistory->_historyName; }
		uint32 getFrameSize()    { return _framesHistory->_frameSize; }
		uint32 getFramesNumber() { return _framesHistory->_framesNumber; }


		void pushFrame(BYTE* frame) {
			_framesHistory->pushFrame(frame);
		}


		/*
		 * Dump the frames history to a file and returns
		 * the filename (HistoryName+dateAndTime)
		 */
		string dumpToFile() {
			_framesHistory->lockBuffer();
			string fileName = Utils::getAdoptLog() + "/" + _framesHistory->getHistoryName() + "_" + Utils::timeAsString() + "_" + Utils::asciiDateAndTimeCompact() + ".log";

            if (Utils::fileExists(fileName)) {
                char n = '0';
                string newfileName;
                do {
                    n++;
                    newfileName = fileName.substr(0,fileName.length()-4) + "_" + n + ".log";
                } while ((Utils::fileExists(newfileName)) && (n != '9'));
                fileName = newfileName;
            }

			Logger::get()->log(Logger::LOG_LEV_INFO, "OFramesHistory::dumpToFile: output file %s", fileName.c_str());

			ofstream dumpFile(fileName.c_str(), ios::out | ios::binary);

			// Dump header
			dumpFile << _framesHistory->getHistoryName() << "\n";
			dumpFile << Utils::itoa((int)_framesHistory->getFrameSize()) << "\n";
			dumpFile << Utils::itoa((int)_framesHistory->getFramesNumber()) << "\n";

			// Dump the two parts to reorder the data
			Logger::get()->log(Logger::LOG_LEV_INFO, "Dumping 1st part of size %d...", _framesHistory->getDataEnd()-_framesHistory->getCursor());
			dumpFile.write((const char*)_framesHistory->getCursor(), _framesHistory->getDataEnd()-_framesHistory->getCursor());
			Logger::get()->log(Logger::LOG_LEV_INFO, "Dumping 2nd part of size %d...", _framesHistory->getCursor()-_framesHistory->getData());
			dumpFile.write((const char*)_framesHistory->getData(), _framesHistory->getCursor()-_framesHistory->getData());

			dumpFile.close();

			// Clean the history
			_framesHistory->resetBuffer();

			_framesHistory->unlockBuffer();
			return fileName;
		}


	private:

		FramesHistory* _framesHistory;
};


/*
 * @Class: IFramesHistory
 * A FramesHistory able to read its content from a file
 * -Implementatio note-
 * The buffer is not fully loaded from the file to save memory resources,
 * but the next frame is retrieved on request (use getFrame() method)
 * @
 */
class IFramesHistory {

	public:

		IFramesHistory(string fileName) {

			string fileNameWithPath = Utils::getAdoptLog() + "/" + fileName;
		 	Logger::get()->log(Logger::LOG_LEV_INFO, "IFramesHistory::contructor: input file %s", fileNameWithPath.c_str());
		 	_historyFileIn = new ifstream(fileNameWithPath.c_str(), ios::in | ios::binary);

		 	// Retrieve this info from file header...
		 	string historyName;
		 	string framesNumberAsString;
		 	string framesSizeBytesAsString;
		 	getline(*_historyFileIn, historyName);
		 	getline(*_historyFileIn, framesSizeBytesAsString);
		 	getline(*_historyFileIn, framesNumberAsString);

		 	// After reading the header, the get pointer points the firts frame!
		 	_curFrameIndex = 0;

		 	uint32 framesNumber = atoi(framesNumberAsString.c_str());
		 	uint32 framesSizeBytes = atoi(framesSizeBytesAsString.c_str());

		 	//... and create the FramesHistory object
		 	_framesHistory = new FramesHistory(historyName, framesNumber, framesSizeBytes);

		 	Logger::get()->log(Logger::LOG_LEV_INFO, "Created IFramesHistory for %d frames", _framesHistory->_framesNumber);

		 	_loadedFrames = new bool[framesNumber];
		 	for(unsigned int i=0; i<framesNumber; i++) {
		 		_loadedFrames[i] = false;
		 	}

		 }


		~IFramesHistory() {
			_historyFileIn->close();
			delete[] _loadedFrames;
			delete _framesHistory;
		}


		string getHistoryName()  { return _framesHistory->_historyName; }
		uint32 getFrameSize()    { return _framesHistory->_frameSize; }
		uint32 getFramesNumber() { return _framesHistory->_framesNumber; }


		BYTE* getFrame(uint32 index) {
			_loadFrame(index);
			return _framesHistory->getFrame(index);
		}


		BYTE* getLastFrame() {
			_loadFrame(_framesHistory->getFramesNumber()-1);
			return _framesHistory->getLastFrame();
		}


	private:

		/*
		 * The wrapped history and its status (mask of frames loaded from file)
		 */
		FramesHistory* _framesHistory;
		bool* _loadedFrames;

		/*
		 * The input file and an index corresponding to the
		 * frame pointed by the ifstream's getPointer
		 */
		ifstream* _historyFileIn;
		int _curFrameIndex;

		/*
		 * Load a frame from file and store it into the buffer
		 */
		void _loadFrame(uint32 index) {

			int offset = index - _curFrameIndex;
			Logger::get()->log(Logger::LOG_LEV_DEBUG, "**************** FRAME READ ****************");
			Logger::get()->log(Logger::LOG_LEV_DEBUG, "Loading frame %d...", index);

			if(index>=_framesHistory->getFramesNumber()) {
				return;
			}

			if(_loadedFrames[index]) {
				Logger::get()->log(Logger::LOG_LEV_DEBUG, "Frame %d already loaded!", index);
				return;
			}

			int frameSize = _framesHistory->getFrameSize();

			// Seek into the input file
			Logger::get()->log(Logger::LOG_LEV_DEBUG, "Current frame: %d (pos: %d)", _curFrameIndex, (int)_historyFileIn->tellg());
			Logger::get()->log(Logger::LOG_LEV_DEBUG, "Offset: %d frames, %d Bytes", offset, offset*frameSize);
			_historyFileIn->seekg(offset*frameSize, ios::cur);
			Logger::get()->log(Logger::LOG_LEV_DEBUG, "Getting frame from location %d...", (int)_historyFileIn->tellg());

			// Get the frame
			BYTE frame[frameSize];
			memset(frame, 0, frameSize);
			_historyFileIn->read((char*)frame, frameSize);
			Logger::get()->log(Logger::LOG_LEV_DEBUG, "New get location is %d", (int)_historyFileIn->tellg());
			_loadedFrames[index] = true;

			// Insert it into the buffer
			_framesHistory->insertFrame(frame, index);

			_curFrameIndex = index+1;	// After the "read" operation the getCursor is at the next frame!
		}
};

}

#endif /*MASTDIAGNLIB_H_INCLUDE*/
