FROM ubuntu:22.04

# Установка базовых зависимостей
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Установка vcpkg
ENV VCPKG_ROOT=/vcpkg
RUN git clone https://github.com/Microsoft/vcpkg.git ${VCPKG_ROOT} \
    && ${VCPKG_ROOT}/bootstrap-vcpkg.sh -disableMetrics

# Установка минимально необходимых зависимостей
RUN ${VCPKG_ROOT}/vcpkg install \
    aws-sdk-cpp[s3] \
    gtest

# Копируем исходный код
WORKDIR /app
COPY . /app

# Сборка с использованием vcpkg
ENV CMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
    && cmake --build .

# Установка переменных окружения для S3
ENV S3_ENDPOINT=minio:9000
ENV S3_ACCESS_KEY=cXGiKi9Otf3xtIbPGHp9
ENV S3_SECRET_KEY=bDpmvYrjsaqPigTazyB9SVQGFxtk9FJeSFVp4nlb

# Запуск тестов
CMD cd build && ctest -V
