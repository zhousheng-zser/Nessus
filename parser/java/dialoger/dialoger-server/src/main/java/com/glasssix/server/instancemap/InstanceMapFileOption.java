package com.glasssix.server.instancemap;

import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import javax.annotation.PostConstruct;
import java.io.*;
import java.util.*;

@Component
public class InstanceMapFileOption {

    private File file = null;
    private Map<String,String> instanceIdMap = null;



    @PostConstruct
    private void init(){
        file = new File("instanceMap.txt");
        System.out.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        System.out.println("file Path: "+file.getAbsolutePath());
        System.out.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        instanceIdMap = new HashMap<>();
        BufferedReader bufferedReader = null;
        try {
            bufferedReader = new BufferedReader(new FileReader(file));
            String line = null;
            while ((line = bufferedReader.readLine()) != null){
                String[] split = line.split(":");
                if(split.length >1) {
                    instanceIdMap.put(split[0], split[1]);
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

    public synchronized void add(String uuid,String instanceId){
        instanceIdMap.put(uuid,instanceId);
        BufferedWriter bufferedWriter = null;
        try {
            bufferedWriter = new BufferedWriter(new FileWriter(file,true));
            if(instanceIdMap.size() !=1) {
                bufferedWriter.newLine();
            }
            bufferedWriter.write(uuid+":"+instanceId);
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

    public synchronized void delete(String uuid){
        if(!instanceIdMap.containsKey(uuid)){
            return;
        }
        instanceIdMap.remove(uuid);
        BufferedWriter bufferedWriter = null;
        try {
            bufferedWriter = new BufferedWriter(new FileWriter(file));
            for(String key: instanceIdMap.keySet()){
                bufferedWriter.write(uuid+":"+instanceIdMap.get(key));
                bufferedWriter.newLine();
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

    public synchronized void update(String uuid,String instanceId){
        instanceIdMap.put(uuid,instanceId);
        BufferedWriter bufferedWriter = null;
        try {
            bufferedWriter = new BufferedWriter(new FileWriter(file));
            for(String key: instanceIdMap.keySet()){
                bufferedWriter.write(key+":"+instanceIdMap.get(key));
                bufferedWriter.newLine();
                bufferedWriter.flush();
            }

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

    public synchronized String getUUId(){
        String uuid = UUID.randomUUID().toString();
        while(instanceIdMap.containsKey(uuid)){
            uuid = UUID.randomUUID().toString();
        }
        return uuid;
    }

    public synchronized String getInstanceId(String uuid){
        return instanceIdMap.get(uuid);
    }

    public synchronized List<String> getInstanceIds(List<String> uuids){
        if(CollectionUtils.isEmpty(uuids)){
            return null;
        }
        List<String> instanceIds = new ArrayList<>();
        for(String uu:uuids){
            if(instanceIdMap.containsKey(uu)){
                instanceIds.add(instanceIdMap.get(uu));
            }
        }
        return instanceIds;
    }
}
