package com.chuangkit.videorenderer;

public class NativeRendererInterface {

    static {
        System.loadLibrary("native-lib");
    }


    /**
     * 进度百分比显示，取值范围为[0-1]
     */
    private static float rendererPercent = 0.0f;

    public NativeRendererInterface() {
        rendererPercent = 0.0f;
    }

    public void setRendererPercent(float rendererPercent) {
        NativeRendererInterface.rendererPercent = rendererPercent;
    }

    /**
     * 获取目前渲染进度的百分比，用于进度条显示。
     *
     * @return 返回当前进度的百分比，范围取值为[0-1]
     */
    public float getRendererPercent() {
        return rendererPercent;
    }

    /**
     * 执行实际的渲染耗时操作，需要调用者单独开辟线程进行工作，防止堵塞UI线程。
     *
     * @param inputJsonFile 输入data.json的全路径，通过该路径判断资源的存在。
     *                      通常是程序的临时目录
     * @param outputVideoFile 输出视频的全路径，渲染程序会将生成的视频写入到指定目录。
     *                        通常是程序的临时目录。
     * @return 如果生成完成返回true,如果中途异常返回false
     */
    public native int rendererVideo(String inputJsonFile, String outputVideoFile);

    /**
     * 中途中断渲染程序，中断程序涉及到资源释放，也需要一定的时间。
     *
     * @return 中断成功返回true
     */
    public native boolean breakRenderer();
}
