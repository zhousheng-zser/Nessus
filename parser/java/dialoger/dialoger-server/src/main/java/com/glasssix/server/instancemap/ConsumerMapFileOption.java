package com.glasssix.server.instancemap;

import org.springframework.stereotype.Component;

import javax.annotation.PostConstruct;
import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@Component
public class ConsumerMapFileOption {
    private File file = null;
    private Map<String, List<String>> consumerMap = null;

    @PostConstruct
    private void init(){
        file = new File("consumerMap.txt");
        consumerMap = new HashMap<>();
        BufferedReader bufferedReader = null;
        try {
            bufferedReader = new BufferedReader(new FileReader(file));
            String line = null;
            while ((line = bufferedReader.readLine()) != null){
                String[] split = line.split(":");
                if(split.length >1) {
                    if(!consumerMap.containsKey(split[0])){
                        consumerMap.put(split[0],new ArrayList<>());
                    }
                    consumerMap.get(split[0]).add(split[1]);
                }
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if(bufferedReader != null){
                try {
                    bufferedReader.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public synchronized void add(String guuidKey,String uuid){
        if(!consumerMap.containsKey(guuidKey)){
            consumerMap.put(guuidKey,new ArrayList<>());
        }
        consumerMap.get(guuidKey).add(uuid);
        BufferedWriter bufferedWriter = null;
        try {
            bufferedWriter = new BufferedWriter(new FileWriter(file,true));
            bufferedWriter.newLine();
            bufferedWriter.write(guuidKey+":"+uuid);
            bufferedWriter.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if(bufferedWriter != null){
                try {
                    bufferedWriter.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public synchronized void delete(String guuidKey,String uuid){
        if(!consumerMap.containsKey(guuidKey)){
            return;
        }
        consumerMap.get(guuidKey).remove(uuid);
        BufferedWriter bufferedWriter = null;
        try {
            bufferedWriter = new BufferedWriter(new FileWriter(file));
            for(String key: consumerMap.keySet()){
                List<String> guidList = consumerMap.get(key);
                for(String id:guidList){
                    bufferedWriter.write(key+":"+id);
                    bufferedWriter.newLine();
                }
            }
            bufferedWriter.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if(bufferedWriter != null){
                try {
                    bufferedWriter.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public synchronized List<String> getUUIDs(String guuidKey){
        return consumerMap.get(guuidKey);
    }
}
