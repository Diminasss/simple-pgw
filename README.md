## Упрощенная модель PGW

Реализовать упрощённую модель PGW (PDN Gateway), которая умеет:

- обрабатывать пользовательский трафик (как uplink, так и downlink);
- создавать и хранить информацию о PDN Connection, EPS Bearers и APN;
- находить абонентов по разным параметрам (таким как: IP Address, выданный абоненту (UE IP Address); CP/DP TEID).

Дополнительно (необязательно) можно реализовать ограничители скорости передачи данных (rate limiter) для каждой сессии
в каждую из сторон отдельно (uplink и downlink).

### Детали реализации

В данном проекте добавлен шаблон, который может помочь выполнить работу. В нем представлены заголовочные файлы классов,
составляющих упрощенную модель PGW. Необходимо по большей части реализовать описанные методы. Также в проекте есть
тесты, проверяющие базовую функциональность, которые также могут помочь лучше понять требуемое поведение.
Сетевое взаимодействие реализовывать не нужно — считаем, что пакеты "приходят извне".

Упрощённая модель PGW поддерживает следующие возможности:
- **Обработка uplink и downlink трафика**:
    - **Uplink**: Пересылка пакетов от пользовательского оборудования (UE) через носитель к шлюзу APN.
    - **Downlink**: Маршрутизация пакетов от APN к Serving Gateway (SGW) через default bearer PDN-соединения.
- **Управление PDN-соединениями**:
    - Создание и удаление PDN-соединений, связанных с APN, адресом SGW и CP TEID.
    - Назначение уникальных IP-адресов для UE.
- **Управление EPS-носителями**:
    - Создание и удаление носителей в рамках PDN-соединения, идентифицируемых DP TEID.
    - Поддержка default и dedicated носителей, где default используется для downlink трафика.
- **Управление APN**:
    - Хранение имён APN и соответствующих им IP-адресов шлюзов.
- **Операции поиска**:
    - Поиск PDN-соединений по CP TEID или IP-адресу UE.
    - Поиск носителей по DP TEID.
- **Тестовое покрытие**:
    - Включает модульные тесты на основе GoogleTest для проверки обработки трафика uplink/downlink, включая случаи с неизвестными носителями или IP-адресами.

## Структура проекта

Проект организован следующим образом:
- `src/` — исходный код модели PGW:
    - `CMakeLists.txt` — конфигурация сборки.
    - `control_plane.h/.cpp` — управляет PDN-соединениями, носителями и APN.
    - `data_plane.h/.cpp` — обрабатывает пакеты uplink и downlink.
    - `pdn_connection.h/.cpp` — представляет PDN-соединение, хранит данные об APN, IP UE, SGW и носителях.
    - `bearer.h/.cpp` — представляет EPS-носитель, связанный с PDN-соединением.
    - `main.cpp` — простой исполняемый файл, демонстрирующий использование модели (требует конкретной реализации `data_plane`).
- `test/` — содержит модульные тесты с использованием GoogleTest:
    - `CMakeLists.txt` — конфигурация сборки.
    - `data_plane_test.cpp` — тесты для проверки обработки пакетов и краевых случаев.
- `CMakeLists.txt` — конфигурация сборки.

## Детали реализации

### Компоненты

#### 1. `control_plane`
Класс `control_plane` управляет состоянием PGW:
- **Структуры данных**:
    - `_pdns`: Хэш-таблица, сопоставляющая CP TEID с PDN-соединениями (`std::unordered_map<uint32_t, std::shared_ptr<pdn_connection>>`).
    - `_pdns_by_ue_ip_addr`: Хэш-таблица для поиска PDN-соединений по IP-адресу UE.
    - `_bearers`: Хэш-таблица, сопоставляющая DP TEID с носителями.
    - `_apns`: Хэш-таблица, связывающая имена APN с их IP-адресами шлюзов.
- **Основные методы**:
    - `find_pdn_by_cp_teid`/`find_pdn_by_ip_address`: Возвращает PDN-соединение по CP TEID или IP UE.
    - `find_bearer_by_dp_teid`: Возвращает носитель по DP TEID.
    - `create_pdn_connection`: Создаёт PDN-соединение для заданного APN, адреса SGW и CP TEID. Использует статический счётчик для генерации уникальных IP-адресов UE (начиная с `10.0.0.1`).
    - `create_bearer`: Создаёт носитель для PDN-соединения, используя входной SGW TEID как DP TEID.
    - `delete_pdn_connection`/`delete_bearer`: Удаляет PDN-соединения или носители, очищая связанные отображения.
    - `add_apn`: Связывает имя APN с IP-адресом шлюза.

**Примечание**: Из-за ограничения на изменение заголовочных файлов TEID берутся из входных параметров (`sgw_cp_teid` как `cp_teid`, `sgw_teid` как `dp_teid`), а IP UE генерируется с помощью статического счётчика.

#### 2. `data_plane`
Класс `data_plane` — абстрактный базовый класс для обработки пакетов:
- **Основные методы**:
    - `handle_uplink`: Находит носитель по DP TEID, получает PDN-соединение и перенаправляет пакет к шлюзу APN через виртуальный метод `forward_packet_to_apn`.
    - `handle_downlink`: Находит PDN-соединение по IP UE, использует default bearer и перенаправляет пакет к SGW через виртуальный метод `forward_packet_to_sgw`.
- **Виртуальные методы**:
    - `forward_packet_to_sgw` и `forward_packet_to_apn` — чисто виртуальные, реализуются в производных классах (например, `mock_data_plane_forwarding` в тестах или `concrete_data_plane` в `main.cpp`).
- **Поведение**:
    - Игнорирует пакеты для неизвестных носителей или IP UE, как проверено в тестах.
    - Использует `control_plane` для поиска носителей и PDN-соединений.

#### 3. `pdn_connection`
Класс `pdn_connection` представляет PDN-соединение:
- **Данные**:
    - `_apn_gateway`: IP-адрес шлюза APN.
    - `_ue_ip_addr`: IP-адрес, назначенный UE.
    - `_cp_teid`: TEID контрольной плоскости.
    - `_sgw_cp_teid` и `_sgw_address`: TEID и адрес SGW.
    - `_bearers`: Хэш-таблица, сопоставляющая DP TEID с носителями.
    - `_default_bearer`: Указатель на default bearer.
- **Основные методы**:
    - `create`: Статический метод для создания `shared_ptr<pdn_connection>`.
    - Геттеры и сеттеры для TEID, IP-адресов и default bearer.
    - `add_bearer`/`remove_bearer`: Управляет картой носителей и обновляет default bearer при необходимости.

#### 4. `bearer`
Класс `bearer` представляет EPS-носитель:
- **Данные**:
    - `_sgw_dp_teid`: TEID данных SGW.
    - `_dp_teid`: TEID данных для носителя.
    - `_pdn`: Ссылка на родительское PDN-соединение.
- **Основные методы**:
    - Геттеры и сеттеры для TEID.
    - `get_pdn_connection`: Возвращает `shared_ptr` на родительское PDN-соединение через `shared_from_this`.

### Обработка трафика
- **Uplink**:
    - Пакет поступает с DP TEID.
    - `data_plane` использует `control_plane` для поиска носителя по DP TEID.
    - Если носитель найден, извлекается PDN-соединение, и пакет перенаправляется к шлюзу APN через `forward_packet_to_apn`.
    - Если носитель или PDN не найдены, пакет игнорируется.
- **Downlink**:
    - Пакет поступает с IP-адресом UE.
    - `data_plane` находит PDN-соединение по IP UE.
    - Если найдено, используется default bearer для пересылки пакета к SGW через `forward_packet_to_sgw`.
    - Если PDN или default bearer не найдены, пакет игнорируется.

### Операции поиска
- **По CP TEID**: `control_plane::find_pdn_by_cp_teid` возвращает PDN-соединение.
- **По IP UE**: `control_plane::find_pdn_by_ip_address` возвращает PDN-соединение.
- **По DP TEID**: `control_plane::find_bearer_by_dp_teid` возвращает носитель.

### Тестовое покрытие
Тесты (`data_plane_test.cpp`) проверяют:
- Пересылку uplink-пакетов к APN для default и dedicated носителей.
- Пересылку downlink-пакетов к SGW через default bearer.
- Обработку неизвестных носителей или IP UE (пакеты игнорируются).
- Корректное хранение и извлечение пакетов в мок-классе (`mock_data_plane_forwarding`).

## Инструкции по сборке и запуску

### Требования
- **CMake**: Версия 3.15 или выше.
- **Компилятор C++**: Поддержка C++23 (например, GCC 13 или новее).
- **Зависимости**:
    - Boost.Asio (версия 1.87, загружается автоматически через CMake).
    - GoogleTest (версия 1.17.0, загружается автоматически через CMake).

## Работа тестов
```bash
diminas@diminas-G5-ME:~/CLionProjects/simple-pgw/cmake-build-release/test$ ./simple_pgw_tests 
Running main() from /home/diminas/CLionProjects/simple-pgw/cmake-build-release/_deps/googletest-src/googletest/src/gtest_main.cc
[==========] Running 5 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 5 tests from data_plane_test
[ RUN      ] data_plane_test.handle_downlink_for_pdn
[       OK ] data_plane_test.handle_downlink_for_pdn (0 ms)
[ RUN      ] data_plane_test.handle_uplink_for_default_bearer
[       OK ] data_plane_test.handle_uplink_for_default_bearer (0 ms)
[ RUN      ] data_plane_test.handle_uplink_for_dedicated_bearer
[       OK ] data_plane_test.handle_uplink_for_dedicated_bearer (0 ms)
[ RUN      ] data_plane_test.didnt_handle_uplink_for_unknown_bearer
[       OK ] data_plane_test.didnt_handle_uplink_for_unknown_bearer (0 ms)
[ RUN      ] data_plane_test.didnt_handle_downlink_for_unknown_ue_ip
[       OK ] data_plane_test.didnt_handle_downlink_for_unknown_ue_ip (0 ms)
[----------] 5 tests from data_plane_test (0 ms total)

[----------] Global test environment tear-down
[==========] 5 tests from 1 test suite ran. (0 ms total)
[  PASSED  ] 5 tests.
```