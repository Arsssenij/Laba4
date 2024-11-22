#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>

// Шаблонный класс-аллокатор, реализующий выделение памяти блоками заданного размера
template <typename T, size_t BlockSize = 10>
class allocatorforme {
public:
    using value_type = T;

    // Конструктор аллокатора
    allocatorforme() : current_block_size(BlockSize), allocated_elements(0), block(nullptr) {
        expand();
    }

    // Деструктор
    ~allocatorforme() {
        for (size_t i = 0; i < allocated_elements; ++i) {
            block[i].~T(); // Явный вызов деструктора для каждого элемента
        }
        std::free(block);
    }

    // Выделяет n элементов
    T* allocate(std::size_t n) {
        if (n > current_block_size) {
            expand();
        }
        return block + allocated_elements;
    }

    // Освобождает память
    void deallocate(T* p, std::size_t n) {
        // Освобождение не реализовано поэлементно, так как память будет освобождена в деструкторе
    }

    // Метод rebind для поддержки разных типов
    template <class U>
    struct rebind {
        typedef allocatorforme<U, BlockSize> other;
    };

private:
    size_t current_block_size;
    size_t allocated_elements;
    T* block;

    // Метод для расширения выделенной памяти
    void expand() {
        size_t new_size = current_block_size * 2;
        T* new_block = static_cast<T*>(std::malloc(new_size * sizeof(T)));

        if (!new_block) {
            throw std::bad_alloc();
        }

        // Копируем старые элементы в новый блок
        if (block) {
            std::memcpy(new_block, block, allocated_elements * sizeof(T));
            std::free(block);
        }

        block = new_block;
        current_block_size = new_size;
    }
};

// Шаблон контейнера, использующий пользовательский аллокатор
template <typename T, size_t MaxSize, typename Allocator = allocatorforme<T>>
class customContainer {
public:
    using value_type = T;

    customContainer() : size(0), data(nullptr) {}

    ~customContainer() {
        if (data) {
            for (size_t i = 0; i < size; ++i) {
                data[i].~T();
            }
            alloc.deallocate(data, size);
        }
    }

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

    T& operator[](size_t index) {
        if (index >= size) {
            throw std::out_of_range("Индекс вне допустимого диапазона");
        }
        return data[index];
    }

    size_t getSize() const {
        return size;
    }

    bool empty() const {
        return size == 0;
    }

private:
    size_t size;
    T* data;
    Allocator alloc;
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
