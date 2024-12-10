package com.example.levitatuor;

import android.content.Intent;
import android.os.Bundle;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.material.navigation.NavigationView;

import androidx.annotation.NonNull;
import androidx.fragment.app.FragmentManager;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AppCompatActivity;

import com.example.levitatuor.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity implements FragmentManager.OnBackStackChangedListener {

    private AppBarConfiguration mAppBarConfiguration;
    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        //t_terminal = new TerminalFragmentUSB();
        //t_devfrag = new DevicesFragment();


        setSupportActionBar(binding.appBarMain.toolbar);
        /*
        binding.appBarMain.fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null)
                        .setAnchorView(R.id.fab).show();
            }
        });
        */
        DrawerLayout drawer = binding.drawerLayout;
        NavigationView navigationView = binding.navView;
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        mAppBarConfiguration = new AppBarConfiguration.Builder(
                R.id.nav_home, R.id.nav_gallery, R.id.nav_slideshow)
                .setOpenableLayout(drawer)
                .build();
        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main);
        NavigationUI.setupActionBarWithNavController(this, navController, mAppBarConfiguration);
        NavigationUI.setupWithNavController(navigationView, navController);


        getSupportFragmentManager().addOnBackStackChangedListener(this);
        if (savedInstanceState == null)
            getSupportFragmentManager().beginTransaction().add(R.id.fragment, new DevicesFragment(), "devices").commit();
        else
            onBackStackChanged();
    }


    @Override
    public void onBackStackChanged() {
        getSupportActionBar().setDisplayHomeAsUpEnabled(getSupportFragmentManager().getBackStackEntryCount() > 0);
    }


    @Override
    protected void onNewIntent(Intent intent) {
        if ("android.hardware.usb.action.USB_DEVICE_ATTACHED".equals(intent.getAction())) {
            TerminalFragmentUSB terminal = (TerminalFragmentUSB) getSupportFragmentManager().findFragmentByTag("terminal");
            if (terminal != null)
                terminal.status("USB device detected");
            TextView mon = findViewById(R.id.serial_monitor_in);
            mon.append("Terminal status: USB device detected!");
        }
        super.onNewIntent(intent);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onSupportNavigateUp() {
        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main);
        return NavigationUI.navigateUp(navController, mAppBarConfiguration)
                || super.onSupportNavigateUp();
    }


    //function for move down on button press
    public void pressMoveDown(@NonNull View v) {
        //if button id == movedown -> number else get number from textbox
        TerminalFragmentUSB t = (TerminalFragmentUSB) getSupportFragmentManager().findFragmentByTag("terminal");
        double mover = 0;

        if (v.equals(findViewById(R.id.button_move_down))) {
            if ((t != null)) {
                t.send(":DO:DOWN");
            } else {
                Toast.makeText(getBaseContext(), "not connected", Toast.LENGTH_SHORT).show();
            }

        } else if (v.equals(findViewById(R.id.affirmMoveNumber))) {
            View inView = findViewById(R.id.numInput);
            EditText inNum = (EditText) inView;
            try {
                mover = Double.parseDouble(inNum.getText().toString());
            } catch (Exception e) {
                Log.e("pressMoveDown", "numInput could not be converted");
            }
            if ((t != null)) {
                t.send(":DO:PA " + (int)(mover + 0.5));
            } else {
                Toast.makeText(getBaseContext(), "not connected", Toast.LENGTH_SHORT).show();
            }
        }
    }

    //function for move up on button press
    public void pressMoveUp(View v) {
        TerminalFragmentUSB t = (TerminalFragmentUSB) getSupportFragmentManager().findFragmentByTag("terminal");
            if ((t != null)) {
                t.send(":DO:UP");
            } else {
                Toast.makeText(getBaseContext(), "not connected", Toast.LENGTH_SHORT).show();
            }

    }


    //function for sending free text @send button
    public void pressSend(View v) {
        View inView = findViewById(R.id.insertFreeText);
        TextView inText = (TextView) inView;
        TerminalFragmentUSB t = (TerminalFragmentUSB) getSupportFragmentManager().findFragmentByTag("terminal");
        String s = inText.getText().toString();
        TextView serialMonitor = findViewById(R.id.serial_monitor_in);
        //serialMonitor.append(s + "\n");
        if (s.equals("clear")) serialMonitor.setText("");
        else {
            if ((t != null)) {
                t.send(s);
            } else {
                Toast.makeText(getBaseContext(), "not connected", Toast.LENGTH_SHORT).show();
            }
        }
        SpannableStringBuilder spn = new SpannableStringBuilder();
        spn.append(s + '\n');
        spn.setSpan(new ForegroundColorSpan(getResources().getColor(R.color.colorSendText)), 0, spn.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        spn.append(serialMonitor.getText());
        if (spn.length() > 1000) spn.delete(1000, spn.length()-1);
        serialMonitor.setText("");
        serialMonitor.append(spn);
    }

    //tries to establish a serial Connection to an Accoustifly
    public void connectSerial(View v) {
        // Find all available drivers from attached devices.
        DevicesFragment f = (DevicesFragment) getSupportFragmentManager().findFragmentByTag("devices");
        try{
            f.refresh();
        }catch (NullPointerException e){
            Toast.makeText(getBaseContext(), "not connected", Toast.LENGTH_SHORT).show();
        }
        toggleConnect(f.replaceTerminal());
    }


    //to toggle the reading of the connect button
    public void toggleConnect(int i) {
        View v = findViewById(R.id.connect);
        Button b = (Button) v;
        if ((i == 0)) {
            b.setText("CONNECTED");
        } else {
            b.setText("DISCONNECTED"); //TODO: set status disconnected as needed
        }
    }



    public void pressIdentification (View v){
        TerminalFragmentUSB t = (TerminalFragmentUSB) getSupportFragmentManager().findFragmentByTag("terminal");
        if ((t != null)) {
            t.send("*IDN?");
            Toast.makeText(getBaseContext(), t.getButtonBuffer(), Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(getBaseContext(), "not connected", Toast.LENGTH_SHORT).show();
        }
    }




    public void pressHelp(View v){
        TerminalFragmentUSB t = (TerminalFragmentUSB) getSupportFragmentManager().findFragmentByTag("terminal");
        if ((t != null)) {
            t.send("HELP?");
            Toast.makeText(getBaseContext(), t.getButtonBuffer(), Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(getBaseContext(), "not connected", Toast.LENGTH_SHORT).show();
        }

    }



    public void pressDiagnostics (View v){
        TerminalFragmentUSB t = (TerminalFragmentUSB) getSupportFragmentManager().findFragmentByTag("terminal");
        if ((t != null)) {
            t.send("DIAG");
            Toast.makeText(getBaseContext(), t.getButtonBuffer(), Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(getBaseContext(), "not connected", Toast.LENGTH_SHORT).show();
        }
    }
}