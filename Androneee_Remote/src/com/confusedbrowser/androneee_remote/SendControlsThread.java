package com.confusedbrowser.androneee_remote;

import android.content.Context;
import android.util.Log;
import java.net.*;

import com.confusedbrowser.androneee_remote.fragment.HudFragment;

/**
 * Thread to communicate to the server.
 * This will send position when the position is changed and 
 * communicate with the server when the position is the same, so
 * the server can know that we are still alive
 *
 * @author David Buttar
 * @author Shahmir Javaid
 */
public class SendControlsThread extends Thread 
{
    /**
     * Context of this object. 
     */
    private Context context;

    /**
     *  Position of the servo.
     *  position - Holds the current position
     *  lastPosition - Holds the last transmitted position 
     */
    private int position = 50;
    private int lastPosition;
    
    /**
     *  Networking variables.
     *   ip - The ip address to connect to
     *   port - The port to connect to
     *   socket - The socket handling wrapper
     */
    private InetAddress ip;
    private int port;
    private DatagramSocket sockHandler;
    
    /**
     * Locking objects.
     * lockPause is used to make the run thread wait and
     * a wrapper to change the pause variable
     * lockPosition is used to lock when getting/setting position
     */
    private Object lockPause = new Object();
    private Object lockPosition = new Object();
    private Object lockIp = new Object();

    /**
     *  Variable controlling the pause state of this thread.
     *  When the onPause function is called this is changed to true
     *  which in turn will call wait on lockPause.
     */
    private boolean pause = false;
    
    /**
     * Variable controlling the execution of a thread.
     * The run() function runs constantly, so exit variable
     * controls the infinate loop
     */
    private boolean exit = false;

    /**
     * Define the gap when the values are resent. We want
     * to ensure that the client communicates with the
     * server in a regular basis, so the server can know
     * the client is still alive. Change this value to determine
     * the milliseconds the client should talk to the server if
     * no new commands are being sent
     */
    private int timeOut = 500;
    
    /**
     * Hold the last time value of the command sent. This will help
     * us determine the last time the value was sent, we can use this
     * for comparison before we can send another value.
     */
    private long timeValueSent = 0; // Time when last value was set

    /**
     * Holds the last time the Bps command was reset. This will help change
     * our view to determine how many bytes have been sent over a second
     * Every time the view is set, the values of this is reset to current time
     */
    private long timeLastBpsReset = 0; // Time when last bps was recorded

    /**
     * The handler to our View. Usefull for when you want the thread
     * to update the view.
     */
    private HudFragment fragment_hud;

    /**
     * Hold the value of bytes sent. It will be reset to 0 after every
     * second, the bps stands for Bytes per Second
     */
    private int bps = 0;

    /**
     * Initiate our thread. Set the variables from the params, and 
     * set our ipAdress object. Also create a new instance of socket 
     *
     * @param context - The context of this thread
     * @param ip - The ip address to send the commands to
     * @param position - The default position which is to be set
     */
    public SendControlsThread(Context context, String ip, int port)
    {
        super();
        Log.d("Androneee", "Androneee ip is:"+ip);
        try
        {
            this.context = context;
            this.lastPosition = this.position;
            this.timeLastBpsReset = System.currentTimeMillis();
            this.setIp(ip);
            this.port = port;
            this.sockHandler = new DatagramSocket();
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * Set our Fragment. 
     *
     * @param fragment_hud - The pointer to the HudFragment object
     */
    public void setHudFragment(HudFragment fragment_hud)
    {
        this.fragment_hud = fragment_hud;
    }   
        
    /**
     * Set the position variable.
     * Lock using lockPosition before setting the this.position
     * value with the parameter
     *
     * @param position - The position to set
     */
    public void setPosition(int position)
    {
        synchronized (lockPosition)
        {
            this.position = position;
        }
    }
    
    /**
     * Set the ip and ipObject.
     * try setting ipObject using an ipAddress. If all is well
     * then also change the ip variable
     *
     * @param ip - The ipaddress in a string
     */
    public void setIp(String ip)
    {
        synchronized (lockIp)
        {
            try
            {
            	Log.d("Androneee", "Androneee ip is:"+ip);
                this.ip = InetAddress.getByName(ip);
                this.fragment_hud.setHudIp(ip);
            }
            catch(Exception e)
            {
                //throw e;
                e.printStackTrace();
            }
        }
    }

    public void setBps(int bps)
    {
        if (bps < 0) bps = 0;

        this.bps = bps;
    }

    public int getBps()
    {
        return this.bps;
    }
    
    /**
     * Set the position. If a position presented is different than
     * the previous position, notify our server over UDP. If we stop sending 
     * signals to our servers the server will revert to fallback mode, to prevent
     * this we periodically send the same signal back to the server to tell it we
     * are still alive.
     *
     * Also if the Thread is paused, our lockPause object will wait. We know the 
     * thread is paused if the @pause variable is true
     */
    public void run() 
    {   
        while(!exit)
        {
            synchronized (lockPosition)
            {
                // Get the current time
                long currentTime = System.currentTimeMillis();
                
                // If the previous position is different or we have reached a timeout
                // send the values to our server
                if(this.position != this.lastPosition || (currentTime-this.timeValueSent > this.timeOut))
                {
                    // Good for debugging
                    //Log.i("position", "Position: " + this.position);

                    try 
                    {
                        synchronized (lockIp)
                        {
                            // first byte sets the protocol and the channel number
                            // second byte will set the position
                            byte command[] = new byte[]{ 33, (byte)(this.position << 1) };

                            // Create the packet
                            DatagramPacket packet = new DatagramPacket(
                                    command, 
                                    command.length, 
                                    this.ip, 
                                    this.port
                                );

                            // Send the packet
                            this.sockHandler.send(packet);

                            // Reset our timeValueSent to now
                            this.timeValueSent = System.currentTimeMillis();

                            // change our lastPosition to the most recent value
                            this.lastPosition = this.position;
                            
                            // Increment our bytes sent
                            this.bps++;
                        }
                    } 
                    catch (Exception e) 
                    {
                        e.printStackTrace();
                    }
                }
            }
            
            // We are in lock state, so sent the thread to wait
            // which can be then woken up by a notify
            synchronized (lockPause) 
            {
                while (pause) 
                {
                    try 
                    {
                        lockPause.wait();
                    } 
                    catch (InterruptedException e) 
                    {
                    }
                }
            }
        }
    }
	
    /**
     * Change the pause flag. When we pause
     * the flag will prevent call lockPause.wait();
     * see run for this behaviour
     */
    public void onPause() 
    {
        synchronized (lockPause) 
        {
            pause = true;
        }
    }

    /**
     * Thread is resumed. Change the pause flag
     * and notify all waiting Objects
     */
    public void onResume() 
    {
        synchronized (lockPause) 
        {
            pause = false;
            lockPause.notifyAll();
        }
    }
}
