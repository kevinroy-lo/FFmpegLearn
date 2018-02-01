package com.tiemagolf.ffmpegplayer;

import android.Manifest;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.Surface;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import java.io.File;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {

    private VideoView videoView;
    private Spinner sp_video;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        videoView = (VideoView) findViewById(R.id.video_view);
        sp_video = (Spinner) findViewById(R.id.sp_video);
        //多种格式的视频列表
        String[] videoArray = getResources().getStringArray(R.array.video_list);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1,
                android.R.id.text1, videoArray);
        sp_video.setAdapter(adapter);
    }

    public void mPlay(View view) {
        int read = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE);
        int write = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (read == PERMISSION_GRANTED && write == PERMISSION_GRANTED) {
            final String video = sp_video.getSelectedItem().toString();
            final String input = new File(Environment.getExternalStorageDirectory(), video).getAbsolutePath();
            //Surface传入到Native函数中，用于绘制
            final Surface surface = videoView.getHolder().getSurface();
            videoView.post(new Thread(new Runnable() {
                @Override
                public void run() {
                    startPlay(input, surface);
                    //startConvertSound(input, surface);
                }
            }));
        } else {
            Toast.makeText(MainActivity.this, "请先授予APP读写内存卡权限", Toast.LENGTH_SHORT).show();
        }
    }

    private native void startPlay(String input, Surface surface);

    private native void startConvertSound(String input, String output);


    static {

        System.loadLibrary("avutil-54");
        System.loadLibrary("swresample-1");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avformat-56");
        System.loadLibrary("swscale-3");
        System.loadLibrary("postproc-53");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("yuv");
        System.loadLibrary("ffmpeg-player");
    }

    public void mSoundDecode(View view) {

        int read = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE);
        int write = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (read == PERMISSION_GRANTED && write == PERMISSION_GRANTED) {
            File mp3File = new File(Environment.getExternalStorageDirectory(), "input.mp4");
            if (!mp3File.exists()) {
                Toast.makeText(MainActivity.this, "请在根目录下放置kongxin.mp3文件", Toast.LENGTH_SHORT).show();
                return;
            }
            final String input = mp3File.getAbsolutePath();
            final String output = new File(Environment.getExternalStorageDirectory(), "kongxin_after.pcm").getAbsolutePath();
            videoView.post(new Thread(new Runnable() {
                @Override
                public void run() {
                    startConvertSound(input, output);
                }
            }));
        } else {
            Toast.makeText(MainActivity.this, "请先授予APP读写内存卡权限", Toast.LENGTH_SHORT).show();
        }
    }


    public AudioTrack createAudioTrack(int sampleRateInHz, int nb_channels) {
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        int channelConfig;
        if (nb_channels == 1) {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        }
        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);

        AudioTrack audioTrack = new AudioTrack(
                AudioManager.STREAM_MUSIC,
                sampleRateInHz, channelConfig,
                audioFormat,
                bufferSizeInBytes, AudioTrack.MODE_STREAM);
        return audioTrack;
    }
}
