#include <iostream>

class StringPointer {
private:
	std::string *ptr;
	std::string *allocated_ptr = NULL;


public:
	
	StringPointer(std::string *Pointer): ptr(Pointer) {}
    
    ~StringPointer() {
    	if (allocated_ptr) {
    		delete allocated_ptr;
    	}
    }

    std::string * operator->() {
    	return getPtrOrAllocate();
    }

    std::string& operator*() {
    	return *getPtrOrAllocate();
    }
    

private:
	std::string* getPtrOrAllocate() {
		if (!ptr) {
    		if (!allocated_ptr) {
    			allocated_ptr = new std::string("");
    		}
    		return allocated_ptr;
    	} else {
    		return ptr;
    	}
	}
};

int main(int argc, char **argv) {
	std::cout << "Started." << std::endl;

	std::string s1 = "Hello, world!";

	StringPointer sp1(&s1);
	StringPointer sp2(NULL);

	std::cout << sp1->length() << std::endl;
	std::cout << *sp1 << std::endl;
	std::cout << sp2->length() << std::endl;
	std::cout << *sp2 << std::endl;
}