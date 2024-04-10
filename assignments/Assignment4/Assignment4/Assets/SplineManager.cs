using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class SplineManager : MonoBehaviour
{
    [SerializeField] GameObject[] splineArray;
    [SerializeField] GameObject splineInstance;
    Vector3 knotPoint;
    Vector3 DEFAULT_START_POINT;
    [SerializeField] int numSplines = 2;
    int timesCreated = 0;

    //TODO: have a way to generate a new spline on click at a specific point

    // Start is called before the first frame update
    void Start()
    {
        DEFAULT_START_POINT = GetComponent<Transform>().position;
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
                    Splines prevSplineRef = splineArray[i - 1].GetComponent<Splines>();
                    splineReference.startPoint.position = prevSplineRef.endPoint.position;
                    splineReference.point2.position = new Vector3(prevSplineRef.point3.position.x, prevSplineRef.point3.position.y, prevSplineRef.point3.position.z * -1);
                    splineReference.point3.position = prevSplineRef.point3.position + new Vector3(9,0,0);
                    splineReference.endPoint.position = prevSplineRef.startPoint.position + new Vector3(9,0,0);
                }


                if (splineInstance != null)
                {
                    splineInstance.GetComponent<Splines>().startPoint.position = splineReference.startPoint.position;
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
            reference.GetComponent<Splines>().startPoint.position = DEFAULT_START_POINT + new Vector3(0, 0, 9);
            reference.GetComponent<Splines>().point2.position = DEFAULT_START_POINT;
            reference.GetComponent<Splines>().point3.position = DEFAULT_START_POINT + new Vector3(9, 0, 9);
            reference.GetComponent<Splines>().endPoint.position = DEFAULT_START_POINT + new Vector3(9, 0, 0);
            splineArray[i] = reference;
        }
    }
}
