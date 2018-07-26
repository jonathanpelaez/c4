//MIT License
//
//Copyright(c) 2018 Alex Kasitskyi
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include <vector>

namespace c4{
    template<typename T>
    class matrix_ref;

    template<typename T>
    class vector_ref {
        friend class matrix_ref<T>;
    private:
        vector_ref(const vector_ref& o) : length(o.length) ptr(o.ptr) {}
        vector_ref& operator=(const vector_ref&) = delete;

        bool is_inside(int x) const {
            return 0 <= x && x < size_;
        }

    protected:
        int size_;
        T* ptr_;

        vector_ref() : size_(0), ptr_(nullptr) {}

    public:
        vector_ref(int length, T* ptr) : size_(length), ptr_(ptr) {}
        vector_ref(std::vector<T>& v) : size_((int)v.size()), ptr_(v.data()) {
            assert(v.size() == (size_t)size_);
        }

        int size() const {
            return size_;
        }

        T* data() {
            return ptr_;
        }

        const T* data() const {
            return ptr_;
        }

        T* begin() {
            return ptr_;
        }

        const T* begin() const {
            return ptr_;
        }

        T* end() {
            return ptr_ + size_;
        }

        const T* end() const {
            return ptr_ + size_;
        }

        T& operator[](int i) {
            assert(is_inside(i));
            return ptr_[i];
        }

        const T& operator[](int i) const {
            assert(is_inside(i));
            return ptr_[i];
        }

        operator T*() {
            return ptr_;
        }

        operator const T*() const {
            return ptr_;
        }

        template<class U>
        explicit operator U*() {
            return reinterpret_cast<U*>(ptr_);
        }

        template<class U>
        explicit operator const U*() const {
            return reinterpret_cast<const U*>(ptr_);
        }

        static const vector_ref<T> create_const(int size, const T* ptr) {
            return vector_ref<T>(size, const_cast<T*>(ptr));
        }
    };

    template<typename T>
	class matrix_ref {
    private:
        matrix_ref(const matrix_ref& o) : height(o.height), width(o.width), stride(o.stride), ptr(o.ptr) {}
        matrix_ref& operator=(const matrix_ref&) = delete;

    protected:
		int height_;
		int width_;
		int stride_;
		T* ptr_;

        matrix_ref() : height_(0), width_(0), stride_(0), ptr_(nullptr) {}

    public:
        matrix_ref(int height, int width, int stride, T* ptr) : height_(height), width_(width), stride_(stride), ptr_(ptr) {}

		int height() const {
			return height_;
		}

		int width() const {
			return width_;
		}

        int stride() const {
			return stride_;
		}

		int stride_bytes() const {
			return stride_ * sizeof(T);
		}

        bool is_inside(int y, int x) const {
            return 0 <= y && y < height_ && 0 <= x && x < width_;
        }

        T* data() {
            return ptr_;
        }

        const T* data() const {
			return ptr_;
		}

        vector_ref<T> operator[](int i) {
            assert(0 <= i && i < height_);
            return vector_ref<T>(width_, ptr_ + i * stride_);
        }

        const vector_ref<T> operator[](int i) const {
            assert(0 <= i && i < height_);
            return vector_ref<T>::create_const(width_, ptr_ + i * stride_);
        }

		const T& clamp_get(int i, int j) const {
            i = std::max(i, 0);
            i = std::min(i, height_ - 1);

            j = std::max(j, 0);
            j = std::min(j, width_ - 1);

			return operator[](i)[j];
		}

        static const matrix_ref<T> create_const(int height, int width, int stride, const T* ptr) {
            return matrix_ref<T>(height, width, stride, const_cast<T*>(ptr));
        }
    };

    template<typename T>
    class __matrix_buffer {
    protected:
        std::vector<T> v;
        __matrix_buffer(size_t size = 0) : v(size) {}
    };

	template<typename T>
	class matrix : private __matrix_buffer<T>, public matrix_ref<T> {
	public:
        matrix(int height, int width, int stride) : __matrix_buffer<T>(height * stride), matrix_ref<T>(height, width, stride, v.data()) {}
        matrix(int height, int width) : matrix(height, width, width) {}
        matrix() : matrix(0, 0, 0) {}
		
        matrix& operator=(const matrix& b) {
            resize(b);
            std::copy(b.v.begin(), b.v.end(), v.begin());

            return *this;
        }

        matrix(const matrix& b) {
            resize(b);
            std::copy(b.v.begin(), b.v.end(), v.begin());
        }

        matrix& operator=(const matrix_ref<T>& b) {
            resize(b);

            for (int i = 0; i < b.height(); i++)
                std::copy(b[i], b[i] + b.width(), (*this)[i]);

            return *this;
        }

        matrix(const matrix_ref<T>& b) {
            resize(b);

            for(int i = 0; i < b.height(); i++)
                std::copy(b[i].data(), b[i].data() + b.width(), (*this)[i].data());
        }

        void resize(int height, int width, int stride){
			this->height_ = height;
			this->width_ = width;
			this->stride_ = stride;
            v.resize(height * stride);
			this->ptr_ = v.data();
		}

		void resize(int height, int width){
			resize(height, width, width);
		}

		void shrink_to_fit(){
            v.shrink_to_fit();
		}

        void clear_and_shrink() {
            resize(0, 0);
            v.shrink_to_fit();
        }

        template<typename T2>
		void resize(const matrix_ref<T2>& b){
			resize(b.height(), b.width(), b.stride());
		}
    };
};
