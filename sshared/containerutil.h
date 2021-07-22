#pragma once
#include <assert.h>
#include <cstddef>

// File contents
// 
// CUArrayAccessor
//   Function pointer to another array's accessor
//   Allows for passing accessors to arrays instead of casting them to a common basetype
//   DO NOT USE PAST THE EXISTANCE OF AN ARRAY!
//   
//   CIterArray<char*> a(monthsOfTheYear, 12);
//   CUArrayAccessor<char*> accessor = a.accessor();
//   printf(accessor[11]); // Will print december
//
// CIterArray
//   Turns a normal array into one that can be iterated over
//   Read only
//
//   CIterArray<int> a(intArray, howManyIntsInTheArray);
//   for(auto i : a)
//       printf("%d\n", i); // Woo prints all numbers in the array! 
//
//
// CSkipArray
//   This is used for easy accessing of elements in a struct as an array
//   Turns something like a vec3 array into an array of just Xs by using a skip of sizeof(y) + sizeof(z)
//   Read only
//
//	 vec3 points[] = {{12,1,9}, {5,2,6}, {7,3,4}};
//   CSkipArray<vec3, float> a(points, points[0].y); // Stride of one vec3 and offset of one float
//   for(int i = 0; i < 3; i++)
//       printf("%d, ", a[i]); // Prints "1, 2, 3,"
//
//   CSkipArray<void, float> a((void*)points, sizeof(glm::vec3), sizeof(float)); // Stride of one vec3 and offset of one float
//   for(int i = 0; i < 3; i++)
//       printf("%d, ", a[i]); // Prints "1, 2, 3,"
//
//
// C2DPXSkipArray
//   Used for skipping around within an array of pointers
//   DO NOT USE WITH NORMAL 2D ARRAYS!
//
//	 vec3** arr = new vec3* [] { new vec3[]{ {0,1,2}, {3,4,5}, {6,7,8}, {9,10,11} }, new vec3[]{ {12,13,14}, {15,16,17}, {18,19,20}, {21,22,23} } };
//	 C2DPXSkipArray<vec3, int> skiparray2dp(arr, arr[1][0].y, 1);
//	 for (int i = 0; i < 4; i++)
//		 printf("%d, ", skiparray2dp[i]); // Prints "13, 16, 19, 22,"



// Function pointer to another array's accessor
// Allows for passing accessors to arrays instead of casting them to a common basetype
// Lets us get by without inheritance for this file which means no slow virtuals :P
template<typename T>
class CUArrayAccessor
{
	typedef T& (*accessor_t)(void*, size_t);
public:
	CUArrayAccessor(T* arr) : m_arr(arr), m_f(&CUArrayAccessor<T>::normalArrayAccessor) { }
	CUArrayAccessor(void* arr, accessor_t f) : m_arr(arr), m_f(f) { }

	T& operator[](size_t i) { return m_f(m_arr, i); }
private:
	static T& normalArrayAccessor(void* arr, size_t i) { return reinterpret_cast<T*>(arr)[i]; }
	void* const m_arr;
	accessor_t const m_f;
};



// Turns a normal array into one that can be iterated over
template<typename T>
class CIterArray
{
	class Iterator;
public:

	CIterArray(T* data, size_t count) : m_elements(data), m_elementCount(count) { }

	inline size_t count() { return m_elementCount; }
	inline T* data() { return m_elements; }

	Iterator begin() { return { m_elements }; }
	Iterator end() { return { m_elements + m_elementCount }; }

	T& operator[](size_t i) { assert(i < m_elementCount); return m_elements[i]; }
	operator T* () const { return m_elements; }

	CUArrayAccessor<T> accessor() const { return { (void*)this, &CIterArray<T>::accessor }; }
	operator CUArrayAccessor<T>() const { return accessor(); }
private:
	static T& accessor(void* t, size_t i) { return ((CIterArray<T>*)t)->operator[](i); }

	class Iterator
	{
		friend CIterArray;
		Iterator(T* p) { m_p = p; }
		T* m_p;
	public:
		Iterator& operator++() { m_p++; return *this; }
		Iterator operator++(int) { Iterator tmp{ m_p }; operator++(); return tmp; }
		bool operator==(const Iterator& rhs) const { return m_p == rhs.m_p; }
		bool operator!=(const Iterator& rhs) const { return m_p != rhs.m_p; }
		T& operator*() { return *m_p; }
	};

	T* const m_elements;
	size_t const m_elementCount;
};

// This is used for easy accessing of elements in a struct as an array
// Turns something like a vec3 array into an array of just Xs by using a skip of sizeof(y) + sizeof(z)

template<typename S, typename T>
class CSkipArray;

template<typename T>
class CSkipArray<void, T>
{
public:
	// Normal array in
	// How many bytes need to be skipped for each element
	// Offset in bytes from start of stride for an element
	CSkipArray(void* data, unsigned int stride, unsigned int offset) : m_data(reinterpret_cast<char*>(data)), m_stride(stride), m_offset(offset) { }

	T& operator[](size_t i) { return *reinterpret_cast<T*>(m_data + m_stride * i + m_offset); }

	CUArrayAccessor<T> accessor() const { return { (void*)this, &CSkipArray<void, T>::accessor }; }
	operator CUArrayAccessor<T>() const { return accessor(); }

protected:
	static T& accessor(void* t, size_t i) { return ((CSkipArray<void, T>*)t)->operator[](i); }

	char* const m_data;
	unsigned int const m_stride;
	unsigned int const m_offset;
};

template<typename S, typename T>
class CSkipArray : public CSkipArray<void, T>
{
public:
	CSkipArray(S* data, unsigned int offset) : CSkipArray<void, T>(data, sizeof(S), offset) { }
	CSkipArray(S* data, T* firstElement) : CSkipArray<void, T>(data, sizeof(S), reinterpret_cast<char*>(firstElement) - reinterpret_cast<char*>(data)) { }
	CSkipArray(S* data, T& firstElement) : CSkipArray<void, T>(data, sizeof(S), reinterpret_cast<char*>(&firstElement) - reinterpret_cast<char*>(data)) { }
};

// A skip array with a specified length
// Can be iterated

template<typename S, typename T>
class CSizedSkipArray;

template<typename T>
class CSizedSkipArray<void, T>
{
	class Iterator;
public:
	// Normal array in
	// How many bytes need to be skipped for each element
	// Offset in bytes from start of stride for an element
	CSizedSkipArray(void* data, unsigned int stride, unsigned int offset, unsigned int elementCount) : m_data(reinterpret_cast<char*>(data)), m_stride(stride), m_offset(offset), m_elementCount(elementCount) { }

	inline size_t count() { return m_elementCount; }

	Iterator begin() { return { *this, 0 }; }
	Iterator end() { return { *this, m_elementCount }; }

	T& operator[](size_t i) { assert(i < m_elementCount); return *reinterpret_cast<T*>(m_data + m_stride * i + m_offset); }


	CUArrayAccessor<T> accessor() const { return { (void*)this, &CSizedSkipArray<void, T>::accessor }; }
	operator CUArrayAccessor<T>() const { return accessor(); }

private:
	static T& accessor(void* t, size_t i) { return ((CSizedSkipArray<void, T>*)t)->operator[](i); }

	// Little odd, but we have to be able to access offset and stride at all times
	// + this means we can error check
	class Iterator
	{
		friend CSizedSkipArray;
		Iterator(CSizedSkipArray<void, T>& a, size_t i) : m_a(a), m_i(i) { }
		CSizedSkipArray<void, T>& m_a;
		size_t m_i;
	public:
		Iterator& operator++() { m_i++; return *this; }
		Iterator operator++(int) { Iterator tmp{ m_a, m_i }; operator++(); return tmp; }
		bool operator==(const Iterator& rhs) const { return m_i == rhs.m_i && &m_a == &rhs.m_a; }
		bool operator!=(const Iterator& rhs) const { return m_i != rhs.m_i || &m_a != &rhs.m_a; }
		T& operator*() { return m_a[m_i]; }
	};

	char* const m_data;
	unsigned int const m_stride;
	unsigned int const m_offset;
	size_t const m_elementCount;
};


template<typename S, typename T>
class CSizedSkipArray : public CSizedSkipArray<void, T>
{
public:
	CSizedSkipArray(S* data, unsigned int offset, size_t elementCount) : CSizedSkipArray<void, T>(data, sizeof(S), offset, elementCount) { }
	CSizedSkipArray(S* data, T* firstElement, size_t elementCount) : CSizedSkipArray<void, T>(data, sizeof(S), reinterpret_cast<char*>(firstElement) - reinterpret_cast<char*>(data), elementCount) { }
	CSizedSkipArray(S* data, T& firstElement, size_t elementCount) : CSizedSkipArray<void, T>(data, sizeof(S), reinterpret_cast<char*>(&firstElement) - reinterpret_cast<char*>(data), elementCount) { }
};


// Used for skipping around within an array of pointers
// DO NOT USE WITH NORMAL 2D ARRAYS!

template<typename S, typename T>
class C2DPXSkipArray;

template<typename T>
class C2DPXSkipArray<void, T>
{
public:
	// Normal array in
	// How many bytes need to be skipped for each element
	// Offset in bytes from start of stride for an element
	// In a 2D array, [x][y], index is x
	C2DPXSkipArray(void** data, unsigned int stride, unsigned int offset, unsigned int index) : m_data(reinterpret_cast<char**>(data)), m_stride(stride), m_offset(offset), m_index(index) { }

	T& operator[](size_t i) { return *reinterpret_cast<T*>(m_data[m_index] + m_stride * i + m_offset); }

	CUArrayAccessor<T> accessor() const { return { (void*)this, &C2DPXSkipArray<void, T>::accessor }; }
	operator CUArrayAccessor<T>() const { return accessor(); }

protected:
	static T& accessor(void* t, size_t i) { return ((C2DPXSkipArray<void, T>*)t)->operator[](i); }

	char** const m_data;
	unsigned int const m_stride;
	unsigned int const m_offset;
	unsigned int const m_index;
};

template<typename S, typename T>
class C2DPXSkipArray : public C2DPXSkipArray<void, T>
{
public:
	// In an array of pointers, [x][y], index is x
	C2DPXSkipArray(S** data, unsigned int offset, unsigned int index) : C2DPXSkipArray<void, T>(data, sizeof(S), offset, index) { }
	C2DPXSkipArray(S** data, T* firstElement, unsigned int index) : C2DPXSkipArray<void, T>((void**)data, sizeof(S), reinterpret_cast<char*>(firstElement) - reinterpret_cast<char*>(data[index]), index) { }
	C2DPXSkipArray(S** data, T& firstElement, unsigned int index) : C2DPXSkipArray<void, T>((void**)data, sizeof(S), reinterpret_cast<char*>(&firstElement) - reinterpret_cast<char*>(data[index]), index) { }
};


// Used for skipping around within an array of pointers
// DO NOT USE WITH NORMAL 2D ARRAYS!

template<typename S, typename T>
class C2DPYSkipArray;

template<typename T>
class C2DPYSkipArray<void, T>
{
public:
	// Normal array in
	// How many bytes need to be skipped for each element
	// Offset in bytes from start of stride for an element
	// In a 2D array, [x][y], index is y
	C2DPYSkipArray(void** data, unsigned int stride, unsigned int offset, unsigned int index) : m_data(reinterpret_cast<char**>(data)), m_stride(stride), m_offset(offset), m_index(index) { }

	T& operator[](size_t i) { return *reinterpret_cast<T*>(m_data[i] + m_stride * m_index + m_offset); }

	CUArrayAccessor<T> accessor() const { return { (void*)this, &C2DPYSkipArray<void, T>::accessor }; }
	operator CUArrayAccessor<T>() const { return accessor(); }

protected:
	static T& accessor(void* t, size_t i) { return ((C2DPYSkipArray<void, T>*)t)->operator[](i); }

	char** const m_data;
	unsigned int const m_stride;
	unsigned int const m_offset;
	unsigned int const m_index;
};

template<typename S, typename T>
class C2DPYSkipArray : public C2DPYSkipArray<void, T>
{
public:
	// In an array of pointers, [x][y], index is y
	C2DPYSkipArray(S** data, unsigned int offset, unsigned int index) : C2DPYSkipArray<void, T>(data, sizeof(S), offset, index) { }
	C2DPYSkipArray(S** data, T* firstElement, unsigned int index) : C2DPYSkipArray<void, T>((void**)data, sizeof(S), reinterpret_cast<char*>(firstElement) - reinterpret_cast<char*>(data[index]), index) { }
	C2DPYSkipArray(S** data, T& firstElement, unsigned int index) : C2DPYSkipArray<void, T>((void**)data, sizeof(S), reinterpret_cast<char*>(&firstElement) - reinterpret_cast<char*>(data[index]), index) { }
};
