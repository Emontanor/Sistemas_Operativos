# Nombre del compilador
CC = gcc

# Flags de compilación
CFLAGS = -lpthread

# Nombre del ejecutable
TARGET = main.exe

# Archivos fuente
SRC = main.c

# Regla principal
all: $(TARGET)

# Compilación del ejecutable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Regla para ejecutar el programa
run: $(TARGET)
	./$(TARGET)

# Limpiar archivos compilados
clean:
	rm -f $(TARGET) *.o