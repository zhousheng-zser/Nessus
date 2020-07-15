package com.glasssix.common.util;

import org.springframework.boot.env.YamlPropertySourceLoader;
import org.springframework.core.env.PropertySource;
import org.springframework.core.io.support.EncodedResource;
import org.springframework.core.io.support.PropertySourceFactory;

import java.io.IOException;
import java.util.List;

public class YamlPropertySourceFactory implements PropertySourceFactory {
    @Override
    public PropertySource<?> createPropertySource(String s, EncodedResource encodedResource) throws IOException {
        List<PropertySource<?>> sources = new YamlPropertySourceLoader().load(encodedResource.getResource().getFilename(), encodedResource.getResource());
        return sources.get(0);
    }
}
