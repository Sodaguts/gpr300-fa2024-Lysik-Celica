using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using Unity.PlasticSCM.Editor.WebApi;
using UnityEngine;

public class Splines : MonoBehaviour
{
    [SerializeField] Transform point1;
    [SerializeField] Transform point2;
    [SerializeField] Transform point3;
    [SerializeField] Transform point4;

    [SerializeField] Transform A;
    [SerializeField] Transform B;
    [SerializeField] Transform C;

    [SerializeField] Transform D;
    [SerializeField] Transform E;

    [SerializeField] Transform Trace;
    Vector3 previousTracePosition;
    Vector3 previousTracePosition2;
    Vector3 previousTracePosition3;
    Vector3 previousTracePosition4;

    public Vector3[] tracePositions;

    public float lerpValue = 0.0f;

    public int amountOfPoints = 4;
    int amountOfPointsCount = 0;
    bool drawGizmosEnabled = false;

    private void Start()
    {
        tracePositions = new Vector3[amountOfPoints];
        drawGizmosEnabled = true;
    }

    private void Update()
    {
        fillTracePointArray();
    }

    private void generateCurve(float _lerpValue, int index) 
    {
        _lerpValue = Mathf.Clamp(_lerpValue, 0.0f, 1.0f);
        if (A != null)
        {
            A.position = Vector3.Lerp(point1.position, point2.position, _lerpValue);
        }
        if (B != null)
        {
            B.position = Vector3.Lerp(point2.position, point3.position, _lerpValue);
        }
        if (C != null)
        {
            C.position = Vector3.Lerp(point3.position, point4.position, _lerpValue);
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
            print(currentLerpValue);
            generateCurve(currentLerpValue, i);
        }
    }


    private void OnDrawGizmosSelected()
    {
        drawBaseStructure(point1, point2, point3, point4);
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
                    Gizmos.DrawLine(point1.position, currentTracePosition);
                }

            }
        }


    }
}
