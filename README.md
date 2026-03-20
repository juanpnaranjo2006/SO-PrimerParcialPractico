# SchedulingSimSO

Simulador de planificación de procesos desarrollado en C++.

---

## 🔹 Instrucciones para ejecutar

1. **Clonar el repositorio**  
   ```bash
   git clone <URL_DEL_REPOSITORIO>
   cd <NOMBRE_DEL_REPOSITORIO>
   ```

2. **Crear la imagen de Docker**
```bash
   docker build -t scheduling-sim .
```

3. **Crear un contenedor interactivo**

```bash
docker run -it --rm scheduling-sim bash
```

Dentro del contenedor, para conocer cómo usar el ejecutable:

```bash
./SchedulingSimSO -h
```

⚡ **Notas**

La imagen de Docker solo incluye el ejecutable compilado SchedulingSimSO y la carpeta ./testFiles.

Puedes ejecutar tus pruebas directamente dentro del contenedor.

Cualquier cambio en los archivos fuente requiere reconstruir la imagen con docker build.
