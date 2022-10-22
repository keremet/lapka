# Компиляция
make

# Сервер
Установка
```
sudo make install-server
```

В файле /lib/systemd/system/lapka-server.service скорректировать имя рабочего каталога

```
sudo systemctl enable --now lapka-server.service
```

# Клиент

Установка
```
sudo make install-client
```

В файле /lib/systemd/system/lapka-client.service указать IP сервера

Ручной запуск клиента
```
sudo systemctl start lapka-client.service
```
Сообщения об ошибках можно посмотреть так:
```
sudo systemctl status lapka-client.service
```
