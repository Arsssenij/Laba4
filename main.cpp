#include <iostream>
#include <map>
#include <memory>
#include <cstddef>


// Шаблонный класс-аллокатор, реализующий выделение памяти блоками заданного размера
// T - тип данных, для которых используется аллокатор
// BlockSize - размер блока (по умолчанию 10)
// stateful аллокатор
template <class T, size_t BlockSize = 10>
struct allocatorforme {
    using value_type = T; // Тип данных, который может хранить аллокатор
    
    void* pool; // Указатель на пул памяти (начало блока)

    // Конструктор аллокатора
    // Инициализирует пул памяти, выделяя блок размера BlockSize * sizeof(T)
    // current_block_size - текущий размер блока
    // allocated_elements - количество выделенных элементов
    // block - указатель на начало блока
    allocatorforme () : current_block_size(BlockSize), allocated_elements(0),
        block(static_cast<T*>(std::malloc(BlockSize * sizeof(T)))) {
        if (!block)
            throw std::bad_alloc();  // Если выделение памяти не удалось, выбрасываем исключение std::bad_alloc
    }
    
    // Деструктор аллокатора
    ~allocatorforme() {
        std::free(block);
    }
    
    // Конструктор копирования
    template <class U> allocatorforme (const allocatorforme<U>&) noexcept {}

    // Выделяет n элементов типа T 
    // Возвращает указатель на выделенную память (использует ::operator new)
    T* allocate (std::size_t n) {
        if (n > current_block_size) {
            throw std::bad_alloc(); // Проверка на превышение размера блока
        }
        return static_cast<T*>(::operator new(n*sizeof(T)));
    }
    


    // Освобождает память, выделенную по указателю p (использует ::operator delete)
    void deallocate (T* p, std::size_t n) { ::operator delete(p); }

    // Метод rebind, который позволяет создавать аллокатор для другого типа данных
    template< class U >
    struct rebind {
        typedef allocatorforme<U> other;
    };
    
    private:
        size_t current_block_size; // текущий размер блока
        size_t allocated_elements; // количество выделенных элементов
        T * block; // указатель на начало блока
};


// T - тип данных, которые хранит контейнер
// MaxSize - максимальное количество элементов в контейнере
// Allocator - тип аллокатора (по умолчанию allocatorforme<T>)
template <typename T, size_t MaxSize, typename Allocator = allocatorforme<T>>
class customContainer {
private:
    Allocator alloc; // Экземпляр аллокатора
    T* data; // Указатель на данные (динамический массив)
    size_t size; // Текущий размер контейнера

public:
    // Конструктор по умолчанию
    customContainer() : alloc(), data(nullptr), size(0) {}
    
    ~customContainer() {
        for (size_t i = 0; i < size; ++i) {
            data[i].~T(); // Явный вызов деструктора для каждого элемента
        }
        alloc.deallocate(data, size);
    }

    // Добавляет элемент в конец контейнера
    void push_back(const T& value) { 
        if (size >= MaxSize) { 
            throw std::runtime_error("Контейнер уже заполнен"); 
        } 
        if (!data) { 
            data = alloc.allocate(MaxSize); 
        } 
        new (&data[size]) T(value); 
        ++size; 
    }

    // Возвращает ссылку на элемент по заданному индексу
    const T& operator[](size_t index) const {
        if (index >= size) {
            throw std::out_of_range("Индекс вне допустимого диапазона");
        }
        return data[index];
    }

    // Возвращает текущий размер контейнера
    size_t getSize() const {
        return size;
    }

    // Проверяет, пуст ли контейнер
    bool empty() const {
        return size == 0;
    }
};

// Функция для вычисления факториала
int factorial(int n) {
    return (n <= 1) ? 1 : n * factorial(n - 1);
}

int main() {
    // 1) Создание экземпляра std::map<int, int>
    std::map<int, int> standard_map;

    // 2) Заполнение 10 элементами, где ключ – это число от 0 до 9, а значение – факториал ключа
    for (int i = 0; i < 10; ++i) {
        standard_map[i] = factorial(i);
    }

    // 3) Создание экземпляра std::map<int, int> с новым аллокатором
    std::map<int, int, std::less<int>, allocatorforme<std::pair<const int, int>>> custom_map;

    // 4) Заполнение 10 элементами, где ключ – это число от 0 до 9, а значение – факториал ключа
    for (int i = 0; i < 10; ++i) {
        custom_map[i] = factorial(i);
    }

    // 5) Вывод на экран всех значений (ключ и значение разделены пробелом), хранящихся в стандартном контейнере
    std::cout << "Стандартный map:\n";
    for (const auto& pair : standard_map) {
        std::cout << pair.first << " " << pair.second << "\n";
    }

    // Вывод на экран всех значений (ключ и значение разделены пробелом), хранящихся в пользовательском контейнере
    std::cout << "Пользовательский map:\n";
    for (const auto& pair : custom_map) {
        std::cout << pair.first << " " << pair.second << "\n";
    }

    // 6) Создание экземпляра своего контейнера для хранения значений типа int
    customContainer<int, 10, std::allocator<int>> my_container;

    // 7) Заполнение 10 элементами от 0 до 9
    for (int i = 0; i < 10; ++i) {
        my_container.push_back(i);
    }

    // Вывод значений из своего контейнера
    std::cout << "Мой контейнер:\n";
    for (size_t i = 0; i < my_container.getSize(); ++i) {
        std::cout << my_container[i] << "\n";
    }

    // 8) Создание экземпляра своего контейнера для хранения значений типа int с новым аллокатором
    customContainer<int, 10, allocatorforme<int>> my_custom_container;

    // 9) Заполнение 10 элементами от 0 до 9
    for (int i = 0; i < 10; ++i) {
        my_custom_container.push_back(i);
    }

    // Вывод значений из своего пользовательского контейнера
    std::cout << "Пользовательский контейнер:\n";
    for (size_t i = 0; i < my_custom_container.getSize(); ++i) {
        std::cout << my_custom_container[i] << "\n";
    }
    return 0;
}