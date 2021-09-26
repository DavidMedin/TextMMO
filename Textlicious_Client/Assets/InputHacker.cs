using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class InputHacker : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        TMP_InputField comp = GetComponent<TMP_InputField>();
        comp.ActivateInputField();
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
