﻿using UnityEngine;
using System.Runtime;
using System.Runtime.InteropServices;
using System;
using System.Collections;
using System.Collections.Generic;
using Ossia;

namespace Ossia
{
  public class Controller : MonoBehaviour
  {
    public string appName = "Unity";
    void Awake ()
    {
      Debug.Log ("OSSIA: Starting");
      if (!set) {
        set = true;

        // Setup the log so that the errors in the C API are shown in the
        // Unity3D console
        callback_delegate = new DebugLogDelegate (DebugLogCallback);

        // Convert callback_delegate into a function pointer that can be
        // used in unmanaged code.
        IntPtr intptr_delegate =
          Marshal.GetFunctionPointerForDelegate (callback_delegate);

        // Call the API passing along the function pointer.
        Ossia.Network.ossia_set_debug_logger (intptr_delegate);

        local_protocol = new Ossia.Local ();
        local_device = new Ossia.Device (local_protocol, "unity");
        scene_node = local_device.GetRootNode ();

        Queue = new Ossia.MessageQueue (local_device);

        oscq_protocol = new Ossia.OSCQuery (1234, 5678);
        local_protocol.ExposeTo (oscq_protocol);


        score_protocol = new Ossia.OSC("127.0.0.1", 6666, 6657);
        score_device = new Ossia.Device(score_protocol, "score");
        
        var reconnect = Ossia.Node.CreateNode(score_device.GetRootNode(), "/reconnect").CreateParameter(Ossia.ossia_type.STRING);
        reconnect.PushValue("");
        score_play = Ossia.Node.CreateNode(score_device.GetRootNode(), "/play").CreateParameter(Ossia.ossia_type.BOOL);
        StartCoroutine(playScore());
      }
    }

        IEnumerator playScore()
        {
            yield return new WaitForSeconds(8);
            score_play.PushValue(true);
        }

    void Update()
    {
      Ossia.Message m;
      while (Queue.Pop (out m)) {
        if (Hash.ContainsKey (m.Address)) {
          Hash [m.Address].ReceiveUpdates ();
        }
      }
    }

    public Ossia.Node SceneNode ()
    {
      return scene_node;
    }

    void OnApplicationQuit ()
    {
      Debug.Log ("OSSIA: Quitting");
      Network.ossia_device_reset_static ();
    }

    public Ossia.Device GetDevice ()
    {
      return local_device;
    }

    static void DebugLogCallback (string str)
    {
      Debug.Log ("OSSIA : " + str);
    }

    internal void Register(Ossia.OssiaEnabledField p)
    {
      Queue.Register (p.ossia_parameter);
      Hash.Add (p.ossia_parameter.ossia_parameter, p);
    }
    internal void Register(Ossia.OssiaEnabledProperty p)
    {
      Queue.Register (p.ossia_parameter);
      Hash.Add (p.ossia_parameter.ossia_parameter, p);
    }
    internal void Unregister(Ossia.OssiaEnabledField p)
    {
      Queue.Unregister (p.ossia_parameter);
      Hash.Remove (p.ossia_parameter.ossia_parameter);
    }
    internal void Unregister(Ossia.OssiaEnabledProperty p)
    {
      Queue.Unregister (p.ossia_parameter);
      Hash.Remove (p.ossia_parameter.ossia_parameter);
    }

        internal void Register(Ossia.OssiaEnabledElement p)
        {
            Queue.Register(p.parameter());
            Hash.Add(p.parameter().ossia_parameter, p);
        }
        internal void Unregister(Ossia.OssiaEnabledElement p)
        {
            Queue.Unregister(p.parameter());
            Hash.Remove(p.parameter().ossia_parameter);
        }

        internal static Controller Get()
    {
      GameObject controller = GameObject.Find ("OssiaController");
      if (controller == null) {
        throw new Exception ("Controller GameObject not found");
      }
      var dev = controller.GetComponent<Ossia.Controller> ();
      if (dev == null) {
        throw new Exception ("Controller component not found");
      }
      return dev;
    }



    bool set = false;

    Ossia.Local local_protocol = null;
    Ossia.Device local_device = null;
    Ossia.OSCQuery oscq_protocol = null;

    Ossia.Device score_device = null;
    Ossia.OSC score_protocol = null;
    Ossia.Parameter score_play = null;
    Ossia.Node scene_node;
    Ossia.Network main;
    Ossia.MessageQueue Queue;
    Dictionary<IntPtr, Ossia.OssiaEnabledElement> Hash = new Dictionary<IntPtr, Ossia.OssiaEnabledElement>();

    public delegate void DebugLogDelegate (string str);

    DebugLogDelegate callback_delegate = null;
  }
}
