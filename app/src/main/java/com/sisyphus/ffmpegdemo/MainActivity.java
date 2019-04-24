package com.sisyphus.ffmpegdemo;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    static final String TAG = MainActivity.class.getName();
    private EditText etFilePath;
    private Button btnDecode;
    private SurfaceView svPreview;
    FFmpegPlayer fFmpegPlayer;
    SeekBar seekBar;

    boolean isOpen = false;
    long duration=0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        seekBar = findViewById(R.id.seekBar);
        etFilePath = findViewById(R.id.et_file_path);
        btnDecode = findViewById(R.id.btn_decode);
        svPreview = findViewById(R.id.sv_preview);

        String filePath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "test.mp4";
        etFilePath.setText(filePath);

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(fFmpegPlayer!=null) {
                    double radio = (int) (progress / 100f * duration);
                    fFmpegPlayer.seekAccurate(radio/1000000);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        btnDecode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final SurfaceHolder surfaceHolder = svPreview.getHolder();

                if (fFmpegPlayer == null) {
                    fFmpegPlayer = new FFmpegPlayer();
                    isOpen = fFmpegPlayer.open(etFilePath.getText().toString());
                    fFmpegPlayer.setSurface(surfaceHolder.getSurface());
                    duration= (long) fFmpegPlayer.getDuration();
                }


//                if(isDecode&&fFmpegPlayer!=null){
//                    fFmpegPlayer.stop();
//                    try {
//                        Thread.sleep(1500);
//                    } catch (InterruptedException e) {
//                        e.printStackTrace();
//                    }
//                }
//
//                final SurfaceHolder surfaceHolder = svPreview.getHolder();
//                new Thread(new Runnable() {
//                    @Override
//                    public void run() {
//                        isDecode=false;
//                        if(fFmpegPlayer==null) {
//                            fFmpegPlayer = new FFmpegPlayer();
//                            isOpen = fFmpegPlayer.open(etFilePath.getText().toString());
//                            fFmpegPlayer.setSurface(surfaceHolder.getSurface());
//                        }
//                        if(isOpen) {
//                            int width = fFmpegPlayer.getWidth();
//                            int height = fFmpegPlayer.getHeight();
//                            String rotate = fFmpegPlayer.getRotate();
//                            long duration = (int) (fFmpegPlayer.getDuration());
//                            Log.d(TAG, duration + "");
//
//
//                                isDecode=true;
//                                fFmpegPlayer.decode(0);
//
//                            isDecode=false;
//                            if(isOnDestory) {
//                                fFmpegPlayer.release();
//                            }
//                        }else{
//                            fFmpegPlayer.release();
//                        }
//                    }
//                }).start();


            }
        });
    }


    boolean isDecode = false;
    boolean isOnDestory = false;

    @Override
    protected void onDestroy() {
        super.onDestroy();
        isOnDestory = true;
        fFmpegPlayer.stop();
        if (!isDecode) {
            fFmpegPlayer.release();
        }
    }
}
