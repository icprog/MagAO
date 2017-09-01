#ifndef STATUSTABLE_H_INCLUDED
#define STATUSTABLE_H_INCLUDED

#include "aotypes.h"
#include "Logger.h"

#include <pthread.h>

using namespace Arcetri;


/*
 * Table summing up the status of a frame buffer.
 * 
 * See MasterDiagnostic class for more info.
 */
class StatusTable {
	
	/*
	 * Inner class defining a single table entry.
	 * 
	 * An entry status is defined by:
	 *  - FrameId: the identifier of the frame wich the entry refers to.
	 *  - ExpectedFrames: the total number of frames expected with the given FrameId.
	 *  - CurExpectedFrames: the number of currently missing frames with the given FrameId.
	 */
	class StatusEntry {
	
		public:
		
			/*
			 * Default constructor - used only to allocate an arrays
			 * of StatusEntry
			 */
			explicit StatusEntry() {}
			
			/*
			 * Constructor
			 */
			explicit StatusEntry(uint32 frameId, uint8 expectedFrames) {
				_frameId = frameId;
				_expectedFrames = expectedFrames;
				_curExpectedFrames = _expectedFrames;
			}
			
			/*
			 * Return the number of expected frames.
			 */
			uint8 getExpectedFrames() { return _expectedFrames; }
			
			/*
			 * Reset the number of expectedFrames to a new
			 * value.
			 */
			void setExpectedFrames(uint8 expectedFrames) {
				_expectedFrames = expectedFrames;
			}
			
			/*
			 * Decrease the entry value corresponding to a frameId, locking
			 * the entry from concurrent access.
			 * 
			 * When the entry's frameId doesn't match the given frameId,
			 * the entry is resetted to the new frameId/curExpectedFrames.
			 * 
			 * Returns the new expectedFrames value.
			 */
			uint8 update(uint32 frameId) {
				if(_frameId == frameId) {
					_curExpectedFrames--;	// Current frame received
					Logger::get()->log(Logger::LOG_LEV_DEBUG, " ---> Updating StatusTable entry for frame %d <---", frameId);
				}
				else {
					_frameId = frameId;
					_curExpectedFrames = _expectedFrames - 1;	//Current NEW frame received !
					Logger::get()->log(Logger::LOG_LEV_DEBUG, " ---> Refreshing StatusTable entry for frame %d <---", frameId);
				}
				return _curExpectedFrames;
			}
			
		
		private:
			
			uint32 _frameId;
			uint8  _expectedFrames;
			uint8  _curExpectedFrames;
	
	};

	public:
	
		explicit StatusTable(string name, uint8 size, uint8 expectedFrames);
		
		virtual ~StatusTable();
		
		/*
		 * Return the number of entries of the table.
		 */
		uint8 getSize() { return _size; }
		
		/*
		 * Set the number of expectedFrames for each ***new*** entry on the table.
		 * This means that only entries "resetted" after this set will be affected by
		 * the new value (see StatusEntry::lockAndUpdate(uint32 frameId) method)
		 */
		void setExpectedFrames(uint8 expectedFrames) { 
			for(int i=0; i<_size; i++) {
				_entries[i].setExpectedFrames(expectedFrames);
			}
			_logger->log(Logger::LOG_LEV_TRACE, "> StatusTable %s: expectedFrames set to %d", _name.c_str(), expectedFrames);
		}
		
		
		// ----------------------- PRODUCER ROUTINES ---------------------- //
		
		/*
		 * Allow a producer client to update a table entry, locking it 
		 * before storing the frame in the buffer.
		 * 
		 * The method return true if the entry is completed, and the lock is mantained.
		 * Returns false otherwise, and the lock is released.
		 * 
		 * If the return value is true, the producer client must call the
		 * method releaseReadyEntry() AFTER the frame is stored.
		 */
		bool updateBufferEntry(int frameId) {
			// Lock the full table for updating, to prevent other producers
			// client from interleaving.
			pthread_mutex_lock(&_mutexUpdating);
		
			if(_entries[(frameId % _size)].update(frameId) == 0) {
				pthread_mutex_lock(&_mutexFrameReady);
				_logger->log(Logger::LOG_LEV_DEBUG, "-> StatusTable %s: entry %d SUCCESFULLY completed", _name.c_str(), frameId % _size);
				_entryReady = frameId % _size;
				return  true;
			}
			// Unlock the table
			else {
				_logger->log(Logger::LOG_LEV_DEBUG, "-> StatusTable %s: entry %d  NOT YET completed", _name.c_str(), frameId % _size);
				return false;
			}
		}
		
		void releaseUpdating() {
			pthread_mutex_unlock(&_mutexUpdating);
		}
		
		/*
		 * Notify a consumer client that an entry (value in _entryReady) is ready.
		 */
		void notifyEntryReady() {
			_logger->log(Logger::LOG_LEV_INFO, "StatusTable %s: notifying for full frame (entry %d) got !", _name.c_str(), _entryReady);
			pthread_cond_signal(&_condFrameReady);
			pthread_mutex_unlock(&_mutexFrameReady);
		}
		
		
		// ----------------------- CONSUMER ROUTINES ---------------------- //
		
		/*
		 * Allow a consumer client to wait for an entry ready.
		 * This method returns the index of the ready entry.
		 * 
		 * After the entry has been used, the client must release
		 * it using the method releaseReadyEntry()
		 */
		int waitReadyEntry() {
			pthread_mutex_lock(&_mutexFrameReady);
			while(_entryReady == -1) {
				pthread_cond_wait(&_condFrameReady, &_mutexFrameReady);
				_logger->log(Logger::LOG_LEV_INFO, "StatusTable %s: waitReadyEntry go !!!", _name.c_str());
				// Here the consumer owns the mutex !
			}
			return _entryReady;
		}
		
		/*
		 * Allow a consumer client to notify it has done with
		 * the ready entry.
		 */
		void releaseReadyEntry() {
			_logger->log(Logger::LOG_LEV_INFO, "StatusTable %s: releasing ready entry %d !", _name.c_str(), _entryReady);
			_entryReady = -1;
			pthread_mutex_unlock(&_mutexFrameReady);
			pthread_mutex_unlock(&_mutexUpdating);
		}
	

	private:
		
		string 		 _name;
		uint8		 _size;
		StatusEntry* _entries;
		
		pthread_mutex_t _mutexUpdating;
		pthread_mutex_t _mutexFrameReady;
		pthread_cond_t  _condFrameReady;
		int				_entryReady;	// Value ranging from 0 to _size, -1 if no entry ready
		
		Logger* _logger;
};

#endif /*STATUSTABLE_H_INCLUDED*/
