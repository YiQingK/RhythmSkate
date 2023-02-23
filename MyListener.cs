/**
 * Ardity (Serial Communication for Arduino + Unity)
 * Author: Daniel Wilches <dwilches@gmail.com>
 * Modifications for InterfaceLab 2020 to move a cube
 *
 * This work is released under the Creative Commons Attributions license.
 * https://creativecommons.org/licenses/by/2.0/
 */
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MyListener : MonoBehaviour
{    
    CharacterController cc;
    bool canmove = true;
    Vector3 movec = Vector3.zero;
    //Line: 0 - Left, 1 - Center, 2 - Right
    int line = 1;
    int targetline = 1;
    public int laneDistance;

    void Start() // Start is called before the first frame update
    {
        cc = this.gameObject.GetComponent<CharacterController> ();
    }
    void Update() // Update is called once per frame
    {
        Move();
        cc.Move(movec*Time.deltaTime*50);
    }

    void checkArduinoInput(float button)
    {
        if(button == 3 && canmove && line>0)
        {
            targetline--;
            canmove = false;
            movec.x = -4;
        }
        if(button == 2 && canmove && line<2)
        {
            targetline++;
            canmove = false;
            movec.x = 4;
        }
    }

    void OnMessageArrived(string msg)
    {
        Debug.Log(msg);
        float button = float.Parse(msg);
        Move();
        checkArduinoInput(button);
        cc.Move(movec*Time.deltaTime*50);
    }

    void Move()
    {
        Vector3 pos = transform.position;
        //Checks current line is not target line
        if(!line.Equals(targetline))
        {
            //Checks if target line is 0
            if(targetline == 0 && pos.x<-laneDistance)
            {
                transform.position = new Vector3(-laneDistance,pos.y,pos.z);
                line = targetline;
                canmove = true;
                movec.x =0;
            }
            //Checks if target line is 1
            else if(targetline== 1 && (pos.x >0 || pos.x <0))
            {
                //Checks if it is currently on 0 
                if(line ==0 && pos.x>0)
                {
                    transform.position = new Vector3(0,pos.y,pos.z);
                    line = targetline;
                    canmove = true;
                    movec.x =0;
                }
                //Checks if it is currently on 2
                else if(line ==2 && pos.x<0)
                {
                    transform.position = new Vector3(0,pos.y,pos.z);
                    line = targetline;
                    canmove = true;
                    movec.x =0;
                }
            }
            //Check if target line is 2
            else if(targetline==2 && pos.x>laneDistance)
            {
                transform.position = new Vector3(laneDistance,pos.y,pos.z);
                line = targetline;
                canmove = true;
                movec.x =0;
            }
        }
    }

    // Invoked when a connect/disconnect event occurs. The parameter 'success'
    // will be 'true' upon connection, and 'false' upon disconnection or
    // failure to connect.
    void OnConnectionEvent(bool success)
    {
        Debug.Log(success ? "Device connected" : "Device disconnected");
    }
}