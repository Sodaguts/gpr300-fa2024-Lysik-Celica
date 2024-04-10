using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SplineManager : MonoBehaviour
{
    [SerializeField] GameObject[] splineArray;
    [SerializeField] GameObject splineInstance;
    Vector3 knotPoint;
    Vector3 DEFAULT_START_POINT = Vector3.zero;
    [SerializeField] int numSplines = 2;
    int timesCreated = 0;

    //TODO: have a way to generate a new spline on click at a specific point

    // Start is called before the first frame update
    void Start()
    {
        splineArray = new GameObject[numSplines];
        fillArray(splineInstance);
    }

    // Update is called once per frame
    void Update()
    {
        if (timesCreated < numSplines) 
        {
            for (int i = 0; i < splineArray.Length; i++)
            {
                Splines splineReference = splineArray[i].GetComponent<Splines>();
                //set the start point of the new spline to the end point of the previous spline (knotPoint)
                if (i > 0)
                {
                    splineReference.startPoint = splineArray[i - 1].GetComponent<Splines>().endPoint;
                    knotPoint = splineReference.startPoint.position;
                }
                else
                {
                    splineReference.startPoint.position = DEFAULT_START_POINT;
                }

                if (splineInstance != null)
                {
                    splineInstance.GetComponent<Splines>().startPoint = splineReference.startPoint;
                    Instantiate(splineInstance);
                    timesCreated++;
                }
            }
        }
    }

    private void fillArray(GameObject reference) 
    {
        for (int i = 0; i < splineArray.Length; i++) 
        {
            splineArray[i] = reference;
        }
    }
}
