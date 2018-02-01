package com.tiemagolf.ffmpegconvert;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Toast;

import com.tiemagolf.ffmpegConvert.R;

import java.io.File;

import static android.support.v4.content.PermissionChecker.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("ffmpeg-convert");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avutil-54");
        System.loadLibrary("postproc-53");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
        final String output = new File(Environment.getExternalStorageDirectory(), "output_yuv420p.yuv").getAbsolutePath();
        findViewById(R.id.sample_text).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int read = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE);
                int write = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
                if (read == PERMISSION_GRANTED && write == PERMISSION_GRANTED) {
                    decode(input, output);
                } else {
                    Toast.makeText(MainActivity.this, "请先授予APP读写内存卡权限", Toast.LENGTH_SHORT).show();
                }

            }
        });
    }

    public native static void decode(String input, String output);

}
