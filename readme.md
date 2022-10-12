# API

1. BMP初始化函数

```
bmp = BMP("ss.bmp")
```

2. getImageHeight 获取图像高度

```
int height = bmp.getImageHeight();
```

3. getImageWidth 获取图像宽度

```
int width = bmp.getImageWidth();
```

4. getDataValue 图像获取(x,y)处的值

```
int value = bmp.getDateValue(x,y);
```

5. setDataValue 图像设置(x,y)处的值

```
bmp.setDataValue(x,y,value);
```

6. Copy 生成一个副本

```
BMP bmpCopy = bmp.Copy();
```