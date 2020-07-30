package com.glasssix.server.instancemap;

import org.springframework.stereotype.Component;

import java.io.*;

@Component
public class InstanceFileOption {

    public synchronized void add(String filename,String context){
        File file = new File("instance/" + filename);
        FileOutputStream fileOutputStream = null;
        try {
            fileOutputStream = new FileOutputStream(file);
            fileOutputStream.write(context.getBytes());
            fileOutputStream.flush();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            try {
                fileOutputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public synchronized void delete(String filename){
        File file = new File("instance/" + filename);
        file.delete();
    }

    public synchronized String getContext(String filename){
        File file = new File("instance/" + filename);
        FileInputStream fileInputStream = null;
        String context = null;
        try{
            byte[] bytes = new byte[(int) file.length()];
            fileInputStream = new FileInputStream(file);
            fileInputStream.read(bytes);
            context = new String(bytes);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            try {
                fileInputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return context;
    }
}
