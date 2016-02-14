
#include <iostream>

#include <list>
#include <algorithm>
#include <sstream>
#include <cstring>

class SmallAllocator {

	//static const unsigned int TOTAL_SIZE = 1048576;
	static const unsigned int TOTAL_SIZE = 1024;

private:
	class Chunk {
	public:
		unsigned int address;
		unsigned int size;
		bool used;

		Chunk(unsigned int address, unsigned int size, bool used)
			: address(address), size(size), used(used) {
			//std::cout << "Created chunk: " << to_string() << std::endl;
		}

		~Chunk() {
			//std::cout << "Deleting chunk: " << to_string() << std::endl;
		}

		std::ostream& operator<<(std::ostream &strm) {
  			return strm << to_string();
		}

		std::string to_string() {
			std::stringstream ss;
			ss << "Chunk(address=" << address << ",size= " << size
  			   <<  (used ? ",used)" : ",free)");
            return ss.str();

		}
	};

	typedef std::list<Chunk>::iterator ChunkIter;

	char memory[TOTAL_SIZE];

	std::list<Chunk> chunks;

public:
	SmallAllocator() {
		Chunk chunk(0, TOTAL_SIZE, false);
		chunks.push_back(chunk);
	}

	~SmallAllocator() {
	}

	void print_state() {
		std::cout << "Chunks: " << std::endl << to_string();
	}

	std::string to_string() {
		std::stringstream ss;
		for (auto iter = chunks.begin(); iter != chunks.end(); ++iter) {
			ss << iter->to_string() << std::endl;
		}
        return ss.str();
	}

	void *Alloc(unsigned int size) {
		if (size <= 0) {
			return NULL;
		}

		void * address = NULL;
		for (ChunkIter iter = chunks.begin(); iter != chunks.end(); ++iter) {
			Chunk &chunk = *iter;
			if (!chunk.used && chunk.size >= size) {
				chunk.used = true;
				address = get_chunk_real_address(chunk.address); 
				if (chunk.size > size) {
					int left_size = chunk.size - size;
					//Chunk chunk_left(chunk.address + size, left_size, false);
					//chunks.insert(++iter, chunk_left);
					chunks.emplace(++iter, chunk.address + size, left_size, false);
					chunk.size = size;
				}
				break;
			}
		}

		return address;
	};

	void *ReAlloc(void *pointer, unsigned int size) {
		if (pointer == NULL) {
			return Alloc(size);
		}
		if (size <= 0) {
			Free(pointer);
			return NULL;
		}

		ChunkIter chunk_iter = chunks.end();
		for (ChunkIter iter = chunks.begin(); iter != chunks.end(); ++iter) {
			//std::cout << "Checking " << iter->address << std::endl;
			if (pointer == get_chunk_real_address(iter->address)) {
				chunk_iter = iter;
				break;
			}
		}

		if (chunk_iter == chunks.end()) {
			std::cerr << "Failed to realloc: no such allocated chunk" << std::endl;
			return NULL;
		}

		ChunkIter next_chunk_iter = chunk_iter;
		++next_chunk_iter;

		if (size == chunk_iter->size) {
			// nothing to change
			return pointer;
		} else if (size < chunk_iter->size) {
			// chunk is shrinked, handle the left memory
			unsigned int freed_size = chunk_iter->size - size;
			chunk_iter->size = size;
			if (next_chunk_iter != chunks.end() && !next_chunk_iter->used) {
				// the next exists and it is free -> extend it
				next_chunk_iter->address = chunk_iter->address + size;
				next_chunk_iter->size = next_chunk_iter->size + freed_size;
			} else {
				// no next chunk to extend -> insert a new after the current
				chunks.emplace(next_chunk_iter, chunk_iter->address + size, freed_size, false);
			}
			return pointer;
		} else {
			// chunk is extended
			unsigned int extra_size = size - chunk_iter->size;
			if (next_chunk_iter != chunks.end() && !next_chunk_iter->used
				&& next_chunk_iter->size >= extra_size) {
				// the next chunk exists, it is free and has enought size -> used it
				if (next_chunk_iter->size == extra_size) {
					// the next chunk is fully used
					chunk_iter->size = size;
					chunks.erase(next_chunk_iter);
				} else {
					// the next chunk is parially used
					chunk_iter->size = size;
					next_chunk_iter->address = chunk_iter->address + size;
					next_chunk_iter->size = next_chunk_iter-> size - extra_size;
				}
				return pointer;
			} else {
				// we have to look for the chunk with appropriate size somewhere else
				// and move data to it
				void *allocated_ptr = Alloc(size);
				if (allocated_ptr == NULL) {
					std::cerr << "ReAlloc failed: no space found" << std::endl;
					return NULL;
				}
				memmove(allocated_ptr, pointer, chunk_iter->size);
				Free(pointer);
				return allocated_ptr;
			}
		}

	};

	void Free(void *pointer) {
		ChunkIter prev_chunk_iter = chunks.end();
		ChunkIter chunk_iter = chunks.end();
		for (ChunkIter iter = chunks.begin(); iter != chunks.end(); ++iter) {
			if (pointer == get_chunk_real_address(iter->address)) {
				chunk_iter = iter;
				break;
			} else {
				// save as a previous and iterate further
				prev_chunk_iter = iter;
			}
		}
		if (chunk_iter == chunks.end()) {
			std::cerr << "Failed to free: no such allocated chunk" << std::endl;
			return;
		}
		if (!chunk_iter->used) {
			std::cerr << "Failed to free: chunk is free" << std::endl;
			return;
		}

		// free the chunk
		chunk_iter->used = false;

		// merge with the previous if possible
		if (prev_chunk_iter != chunks.end() && !prev_chunk_iter->used) {
			merge_chunk_with_prev(chunk_iter, prev_chunk_iter);
		}

		prev_chunk_iter = chunk_iter;
		++chunk_iter;
		if (chunk_iter != chunks.end() && !chunk_iter->used) {
			merge_chunk_with_prev(chunk_iter, prev_chunk_iter);
		}

	};


private:

	void * get_chunk_real_address(int chunk_address) {
		return ((void *)&memory) + chunk_address;
	}

	void merge_chunk_with_prev(ChunkIter chunk_iter, ChunkIter prev_chunk_iter) {
		chunk_iter->address = prev_chunk_iter->address;
		chunk_iter->size = prev_chunk_iter->size + chunk_iter->size;
		chunks.erase(prev_chunk_iter);
	}
};

int main(int argc, char **argv) {
	std::cout << "Allocator test started" << std::endl;
	SmallAllocator small_allocator;
	void * ptr0 = small_allocator.Alloc(512);
	small_allocator.Alloc(64);
	small_allocator.Alloc(64);
	void * ptr1 = small_allocator.Alloc(64);
	void * ptr2 = small_allocator.Alloc(64);
	void * ptr3 = small_allocator.Alloc(64);
	void * ptr4 = small_allocator.Alloc(64);
	small_allocator.Alloc(64);
	small_allocator.Alloc(64);
	small_allocator.print_state();

	small_allocator.Free(ptr1);
	small_allocator.print_state();

	small_allocator.Free(ptr3);
	small_allocator.print_state();

	small_allocator.Free(ptr4);
	small_allocator.print_state();

	small_allocator.Free(ptr2);
	small_allocator.print_state();

	void * ptr5 = small_allocator.Alloc(200);
	small_allocator.print_state();

	small_allocator.Free(ptr0);
	small_allocator.print_state();

	ptr5 = small_allocator.ReAlloc(ptr5, 220);
	small_allocator.print_state();

	ptr5 = small_allocator.ReAlloc(ptr5, 150);
	small_allocator.print_state();

	ptr5 = small_allocator.ReAlloc(ptr5, 256);
	small_allocator.print_state();

	ptr5 = small_allocator.ReAlloc(ptr5, 300);
	small_allocator.print_state();
}