package com.glasssix.init;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.protocol.ProtocolCache;
import com.glasssix.protocol.ProtocolCommon;

public class InitAlgorithmProtocol {

    public static AlgorithmFactory initProtocol(String protocolConfigFileName,String libName){
        AlgorithmFactory algorithmFactory = new AlgorithmFactory(libName);
        ProtocolCommon.setAlgorithmFactory(algorithmFactory);
        new ProtocolCache(protocolConfigFileName);
        return algorithmFactory;
    }
}
