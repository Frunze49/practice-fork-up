import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Type;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.List;

public class Main {
    public static class Student {
        private String name;
        private String login;
        private String group;
        private byte[] practice;
        private Project project;
        private float mark;
    }

    public static class Project {
        private String repo;
        private byte mark;
    }

    public static void main(String[] args) {
        if (args.length != 1) {
            fail("main");
        }
        String fileName = args[0];
        Path inputFilePath = Path.of(fileName);
        if (fileName.endsWith(".bin")) {
            Path jsonFilePath = Path.of(fileName.substring(0, fileName.length() - ".bin".length()) + ".json");
            formatBinToJson(inputFilePath, jsonFilePath);
        } else if (fileName.endsWith(".json")) {
            Path binFilePath = Path.of(fileName.substring(0, fileName.length() - ".json".length()) + ".bin");
            formatJsonToBin(inputFilePath, binFilePath);
        } else {
            fail("main");
        }
    }
    public static void formatBinToJson(Path binFilePath, Path jsonFilePath) {
        System.out.println("Reading binary student data from " + binFilePath + "...");
        List<Student> students = readBinFile(binFilePath);
        if (students != null) { // Проверяем на null
            System.out.println(students.size() + " students read...");
            writeJsonFile(jsonFilePath, students);
            System.out.println("written to " + jsonFilePath);
        } else {
            System.out.println("Failed to read students from JSON file.");
        }
    }
    public static void formatJsonToBin(Path jsonFilePath, Path binFilePath) {
        System.out.println("Reading json student data from " + jsonFilePath + "...");
        List<Student> students = readJsonFile(jsonFilePath);
        if (students != null) { // Проверяем на null
            System.out.println(students.size() + " students read...");
            writeBinFile(binFilePath, students);
            System.out.println("written to " + binFilePath);
        } else {
            System.out.println("Failed to read students from JSON file.");
        }
    }

    public static List<Student> readJsonFile(Path filePath) {
        try {
            Gson gson = new Gson();
            Type type = new TypeToken<List<Student>>() {
            }.getType();
            return gson.fromJson(Files.readString(filePath), type);
        } catch (IOException ex) {
            fail("readJsonFile");
            return null; // Возвращаем null в случае ошибки чтения файла
        }
    }

    public static void writeJsonFile(Path filePath, List<Student> students) {
        try {
            Gson gson = new Gson();
            Files.writeString(filePath, gson.toJson(students));
        } catch (IOException ex) {
            fail("writeJsonFile");
        }
    }

    public static List<Student> readBinFile(Path filePath) {
        try (FileChannel fc = FileChannel.open(filePath, StandardOpenOption.READ)) {
            long fcSize = fc.size();
            ByteBuffer buffer = ByteBuffer.allocate((int) fcSize);
            buffer.order(ByteOrder.LITTLE_ENDIAN);
            while (buffer.hasRemaining()) {
                fc.read(buffer);
            }
            buffer.flip();

            List<Student> students = new ArrayList<>();
            while (buffer.remaining() >= 128) {
                Student student = new Student();
                student.name = getString(buffer, 32);
                student.login = getString(buffer, 16);
                student.group = getString(buffer, 8);
                student.practice = getBytes(buffer, 8);
                student.project = new Project();
                student.project.repo = getString(buffer, 59);
                student.project.mark = buffer.get();
                student.mark = buffer.getFloat();
                students.add(student);
            }
            return students;
        } catch (IOException ex) {
            ex.printStackTrace();
            fail("readBinFile");
            return null; // Возвращаем null в случае ошибки чтения файла
        }
    }

    public static String getString(ByteBuffer buffer, int length) {
        String string = new String(getBytes(buffer, length));
        return string.replace("\0", "");
    }

    public static byte[] getBytes(ByteBuffer buffer, int length) {
        byte[] dst = new byte[length];
        buffer.get(dst, 0, length);
        return dst;
    }

    private static void writeBinFile(Path filePath, List<Student> students) {
        try {
            ByteBuffer buffer = ByteBuffer.allocate(students.size() * 128);
            buffer.order(ByteOrder.LITTLE_ENDIAN);
            for (Student st : students) {
                putStringBytes(buffer, st.name, 32);
                putStringBytes(buffer, st.login, 16);
                putStringBytes(buffer, st.group, 8);
                buffer.put(st.practice, 0, 8);
                putStringBytes(buffer, st.project.repo, 59);
                buffer.put(st.project.mark);
                buffer.putFloat(st.mark);
            }
            buffer.flip();

            FileChannel fc = new FileOutputStream(filePath.toFile()).getChannel();
            fc.write(buffer);
            fc.close();
        } catch (Exception ex) {
            fail("writeBinFile");
        }
    }

    public static void putStringBytes(ByteBuffer buffer, String string, int length) {
        StringBuilder builder = new StringBuilder(string);
        while (builder.length() < length) {
            builder.append("\0");
        }
        buffer.put(builder.toString().getBytes(), 0, length);
    }

    public static void fail(String where) {
        // uncomment for fuzzing
        System.err.printf("Malformed input at %s\n", where);
        throw new IllegalArgumentException("Malformed input at " + where);

        // comment for fuzzing
//        System.err.printf("Malformed input at %s\n", where);
//        System.exit(-1);
    }
}
