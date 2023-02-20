#ifndef POOL_H
#define POOL_H
#endif
#include <cstddef>
#include <new>
#include <iostream>
#include <stdio.h>
#include <string.h>

using std::size_t;
using std::cout;
using std::endl;
using std::byte;

template <typename T>
class Pool
{
	union element {
		byte object[sizeof(T)];
		element* next;
	};
	
	const size_t _blockSize;
	const bool	_trace;
	
	element*	_freehead;
	element**	_thepool;
	
	int		_num_blocks;
	
	void expand();		// Add more blocks
	
	void link(element*);

	int getFree() const;
	
public:

	Pool(size_t blockSize = 5, bool traceFlag = true) :
	_blockSize (blockSize), _trace (traceFlag),
	_freehead (nullptr),
	_thepool (nullptr),
	_num_blocks (0)
{}
   ~Pool();
   void* allocate();       // Get a pointer inside a pre-allocated block for a new object
   void deallocate(void*); // Free an object's slot (push the address on the "free list")
   void profile();   // Print the addresses on the free list.
};
	
// Add the member function definitions here

/* expands the current pool by an additional block of memory to be 
 * able to hold _num_blocks more instances of the provided object
 * 
 */
template <typename T>
void Pool<T>::expand() {
	std::cout << "Expanding pool..." << endl;

	element** temp = new element*[_num_blocks+1];

	std::copy(_thepool, _thepool + (_num_blocks), temp);

	_thepool = temp;
	
	_thepool[_num_blocks] = new element[_blockSize];

	cout << "Linking cells starting at 0x" << (*(_thepool + (_num_blocks))) << endl;

	for (size_t i = _blockSize; i > 0; i--) {	//move through new block backwards adding to freelist
		element* next = (*(_thepool + (_num_blocks)) + i);
		link(next);
	}
	++_num_blocks;
}

/* adds a new block of memory to the linked 'free' list, allowing it 
 * to be used for a new instance of the object
 *
 */
template <typename T>
void Pool<T>::link(element* newHead) {
	newHead->next = _freehead;
	_freehead = newHead;
}

/* returns the number of free memory blocks in the linked 'free' list
 *
 */
template <typename T>
int Pool<T>::getFree() const {
	int freeNum = 0;
	element* nextCell = _freehead;
	while (nextCell != 0) {
		freeNum++;
		nextCell = nextCell->next;
	}
	return freeNum;
}

/* delete function for the entire pool of memory blocks, 
 * deletes each object instance
 *
 */
template <typename T>
Pool<T>::~Pool() {
	cout << "Deleting " << _num_blocks << " blocks" << endl;
	delete[] _thepool;
}

/* to be called as a custom object's 'new' function, handles memory more
 * efficiently than default 'new' calls
 *
 */
template <typename T>
void* Pool<T>::allocate() {
	if (_freehead == 0)
		expand();
	cout << "Cell allocated at 0x" << _freehead << endl;
	element* oldHead = _freehead;
	_freehead = _freehead->next;
	return oldHead;
}

/* deletes an object in the pool's memory, adds memory location to
 * the 'free' list 
 *
 */
template <typename T>
void Pool<T>::deallocate(void* cell) {
	element* newHead = static_cast<element*>(cell);
	std::cout << "Cell deallocated at 0x" << newHead << endl;
	newHead->next = _freehead;
	_freehead = newHead;
}

/* provides a basic rundown on the pool's current state
 *
 */
template <typename T>
void Pool<T>::profile() {
	int total = this->_num_blocks * _blockSize;
	cout << "total " << total << endl;
	int free = getFree();
	std::cout << "Live Cells: " << total - free << ", Free Cells: " << free << endl;
	std::cout << "Free List:" << endl;
	element* nextCell = _freehead;
	while (nextCell != 0) {
		std::cout << "0x" << nextCell << endl;
		nextCell = nextCell->next;
	}
}