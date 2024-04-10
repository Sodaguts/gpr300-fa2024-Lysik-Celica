using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using Unity.PlasticSCM.Editor.WebApi;
using UnityEditor;
using UnityEngine;

public class Splines : MonoBehaviour
{
    [SerializeField] public Transform startPoint;
    [SerializeField] public Transform point2;
    [SerializeField] public Transform point3;
    [SerializeField] public Transform endPoint;

    [SerializeField] Transform A;
    [SerializeField] Transform B;
    [SerializeField] Transform C;

    [SerializeField] Transform D;
    [SerializeField] Transform E;

    [SerializeField] Transform Trace;

    public Vector3[] tracePositions;

    public float lerpValue = 0.0f; // T

    public int amountOfPoints = 1000;
    private int min_amountOfPoints = 1;
    private int max_amountOfPoints;

    bool drawGizmosEnabled = false;

    public void createNewSpine() 
    {

    }

    private void Start()
    {
        max_amountOfPoints = amountOfPoints;
        tracePositions = new Vector3[amountOfPoints];
        drawGizmosEnabled = true;
    }

    private void Update()
    {
        fillTracePointArray();
        amountOfPoints = Mathf.Clamp(amountOfPoints, min_amountOfPoints, max_amountOfPoints);
    }

    private void generateCurve(float _lerpValue, int index) 
    {
        _lerpValue = Mathf.Clamp(_lerpValue, 0.0f, 1.0f);
        if (A != null)
        {
            A.position = Vector3.Lerp(startPoint.position, point2.position, _lerpValue);
        }
        if (B != null)
        {
            B.position = Vector3.Lerp(point2.position, point3.position, _lerpValue);
        }
        if (C != null)
        {
            C.position = Vector3.Lerp(point3.position, endPoint.position, _lerpValue);
        }

        if (D != null)
        {
            D.position = Vector3.Lerp(A.position, B.position, _lerpValue);
        }
        if (E != null)
        {
            E.position = Vector3.Lerp(B.position, C.position, _lerpValue);
        }
        if (Trace != null) 
        {
            Trace.position = Vector3.Lerp(D.position, E.position, _lerpValue);
        }

        tracePositions[index] = Trace.position; 

    }

    private void fillTracePointArray()
    {
        float pointLerpValue = (float)(1.0 / amountOfPoints);
        float currentLerpValue = 0;
        for (int i = 0; i < amountOfPoints; i++)
        {
            currentLerpValue = currentLerpValue + pointLerpValue;
            //print(currentLerpValue);
            generateCurve(currentLerpValue, i);
        }
    }


    private void OnDrawGizmos()
    {
        drawBaseStructure(startPoint, point2, point3, endPoint);
        //drawBlueLines();
        //drawRedLines();
        drawTraceLine();
    }

    private void drawBaseStructure(Transform p1, Transform p2, Transform p3, Transform p4) 
    {
        if (p1 != null && p2 != null && p3 != null && p4 != null) 
        {
            Gizmos.color = Color.white;
            Gizmos.DrawLine(p1.position, p2.position);
           // Gizmos.DrawLine(p2.position, p3.position);
            Gizmos.DrawLine(p3.position, p4.position);
        }
    }

    private void drawBlueLines() 
    {
        if (A != null && B != null && C != null) 
        {
            Color lightBlue = new Color((float)0.0141, (float)0.7962, (float)1.0);
            Gizmos.color = lightBlue;
            Gizmos.DrawLine(A.position, B.position);
            Gizmos.DrawLine(B.position, C.position);
        }
    }

    private void drawRedLines() 
    {
        if (D != null && E != null) 
        {
            Gizmos.color = Color.red;
            Gizmos.DrawLine(D.position, E.position);
        }
    }


    private void drawTraceLine() 
    {
        Gizmos.color = Color.black;

        if (drawGizmosEnabled) 
        {
            for (int i = 0; i < amountOfPoints; i++)
            {
                Vector3 currentTracePosition = tracePositions[i];
                if (i > 0)
                {
                    Gizmos.DrawLine(tracePositions[i - 1], currentTracePosition);
                }
                else
                {
                    Gizmos.DrawLine(startPoint.position, currentTracePosition);
                }

            }
        }


    }
}
