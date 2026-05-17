# KeyBox 🔑

(*Для того чтобы принять участие в обсуждении проекта можете перейти по ссылке https://github.com/Bambuchachkin/KeyBox/discussions/1)

## Цель проекта 🎯
Создать устройство, предназначенное для одновременного хранения ~ 10 ключей от
аудиторий в корпусах МФТИ. Оснастить его системой автоматического контроля
доступа к ключам через карты-пропуска. Вести учёт пользователей и времени, которое
ключи были у них на руках.

## Итоги работы ✨

### По итогам работы нашей команде удалось выполнить все поставленные цели:
1. ✅ Создано устройство для хранения 10 ключей
2. ✅ Установлен замок для ограничения доступа к ключам
3. ✅ Инсталирована светодиодная лента для улучшения пользовательского опыта
4. ✅ Реализована работа с картами-пропусками
5. ✅ Спроектирована база данных пользователей
6. ✅ Налажена работа с таблицами формата .xlsx для удобства работы с устройством
7. ✅ Разработаны методы работы с устройством с помощью консоли
8. ✅ Разработано и произведено функциональное устройство

<p align="center">
  <img src="Documents/Images/Box_1.jpg" alt="ESP32 Tetris Console" width="500">
</p>
<p align="center">
  <img src="Documents/Images/Box_2.jpg" alt="ESP32 Tetris Console" width="500">
</p>

### Решенные проблемы: 🔧
В процессе работы возникали проблемы различного уровня сложности. Среди наиболее острых, но разрешенных оказались:
1. Механизм открытия и закрытия двери, требующий от пользователя минимум действий, а именно приложить карту-пропуск и захлопнуть дверцу после взятия/возвращения ключа. Механизм был реализован при помощи системы: сервопривод, штифты и клинья, возвратные пружины.
2. Реализация наиболее удобных способов взаимодействия персонала с устройством. Такими являются консольное взаимодействие для быстрой регистрации и редактирования профилей пользователей и exel-таблицы для обработки базы данных и журнальных записей (приходы, уходы, взятые ключи).

### Возможное развитие проекта: 🚀
Полученное изделие имеет массу возможностей для улучшения:
1. Дистанционное взаимодействие посредством сети wi-fi
2. Оповещающие устройства (сигнальный динамик, информирующий дисплей)
3. Система автономного реагирования (автоматические ограничения для недобросовестных пользователей)

## Далее вы можете более подробно ознакомиться с историей развития проекта 📖

### Принципиальная схема цепи и взаимодействия ее компонент:

<p align="center">
  <img src="Documents/Images/Scheme.png" alt="ESP32 Tetris Console" width="500">
</p>
<p align="center">
  <img src="Documents/Images/Logic.png" alt="ESP32 Tetris Console" width="500">
</p>

## Ход работы

### Особенности физтеховских карт-пропусков:
<table>
<tr>
<td style="border: none;">

Наименование: mifare classic 1k

Рабочая частота: 13.56 МГц

Требует аутентификации: Для данных, но не для UID

</td> </tr> </table>

### Взаимодействие с картой:
Общение с картами будет осуществлятся при помощи считывателя RC522.
<p align="center">
  <img src="Documents/Images/Logic.png" alt="ESP32 Tetris Console" width="500">
</p>
<p align="center">
  <img src="Documents/Images/RC522.png" alt="ESP32 Tetris Console" width="500">
</p>
<!-- ![Отрисовка карты.](Documents/Images/RC522.png) -->

### Работа в материале

Схема работы детектирующего устройства:
<p align="center">
  <img src="Documents/Images/Hook.png" alt="ESP32 Tetris Console" width="500">
</p>

Модель корпуса ключницы:
<p align="center">
  <img src="Documents/Images/Box.jpg" alt="ESP32 Tetris Console" width="500">
</p>

### Написание кода
Реализация взаимодействия с RC522:
<p align="center">
  <img src="Documents/Images/RFID.png" alt="ESP32 Tetris Console" width="500">
</p>

Реализация обработки команд с консоли:
<p align="center">
  <img src="Documents/Images/ProcessCommand.png" alt="ESP32 Tetris Console" width="500">
</p>

Реализация работы с пользователями:
<p align="center">
  <img src="Documents/Images/Users.png" alt="ESP32 Tetris Console" width="500">
</p>

Разработка структуры хранения данных:
<p align="center">
  <img src="Documents/Images/Table.png" alt="ESP32 Tetris Console" width="500">
</p>

Реализация чтения и записи данных в таблицу (тестовые отправки):
<p align="center">
  <img src="Documents/Images/json_exchange.png" alt="ESP32 Tetris Console" width="500">
</p>

Реализация хранения и восстановления данных из постоянной памяти:
<p align="center">
  <img src="Documents/Images/data_recovery.png" alt="ESP32 Tetris Console" width="500">
</p>

<!-- Добавлен кармашек для  зажима микропереключателя:
<img width="1541" height="1035" alt="image" src="https://github.com/user-attachments/assets/2c278192-2b9a-40d1-a9de-ddc6e570d0b0" />

Доработана основная платформа для закрепления кармашка в ней, так же увеличен радиус отерстия для пружинки:
<img width="1212" height="1104" alt="image" src="https://github.com/user-attachments/assets/5a2353b9-1f5b-4b1b-9aa3-503ce4ea96b2" /> -->

Сделан корпус из дерева (дополнительные отверстия будут делаться на физтех-фабрике)
<p align="center">
  <img src="https://github.com/user-attachments/assets/810f8d19-dff7-42e2-9041-3b519ff64c55" alt="ESP32 Tetris Console" width="500">
</p>
<!-- <img width="767" height="1207" alt="image" src="https://github.com/user-attachments/assets/810f8d19-dff7-42e2-9041-3b519ff64c55" /> -->
<!-- 
Реализация чтения данных пользователей из таблицы (предитоговая версия):
![Отрисовка карты.](Documents/Images/json_read.png) -->


Окончательная реализация чтения данных пользователей из таблицы:
<p align="center">
  <img src="Documents/Images/json_read_final.png" alt="ESP32 Tetris Console" width="500">
</p>

Сборка и тестирование 2ух узлов схемы + отладка кода:
<p align="center">
  <img src="Documents/Images/Test_Circuit.jpg" alt="ESP32 Tetris Console" width="500">
</p>
