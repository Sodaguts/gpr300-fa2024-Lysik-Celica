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
    public List<Vector3> tracePoints = new List<Vector3>();

    private void fillTracePointArray() 
    {
        
        for (int i = 0; i < amountOfPoints; i++) 
        {
            //tracePoints.Add(Vector3.Lerp(Vector3.Lerp(Vector3.Lerp(point1.position, point2.position, lerpValue - (float)(i / amountOfPoints)), Vector3.Lerp(point2.position, point3.position, lerpValue - (float)(i / amountOfPoints)), lerpValue - (float)(i / amountOfPoints)), Vector3.Lerp(Vector3.Lerp(point2.position, point3.position, lerpValue - (float)(i / amountOfPoints)), Vector3.Lerp(point3.position, point4.position, lerpValue - (float)(i / amountOfPoints)), lerpValue - (float)(i / amountOfPoints)), lerpValue - (float)(i / amountOfPoints)));
        }
    }

    private void Start()
    {

    }

    private void Update()
    {
        lerpValue = Mathf.Clamp(lerpValue, 0.0f, 1.0f);
        if (A != null)
        {
            A.position = Vector3.Lerp(point1.position, point2.position, lerpValue);
        }
        if (B != null)
        {
            B.position = Vector3.Lerp(point2.position, point3.position, lerpValue);
        }
        if (C != null)
        {
            C.position = Vector3.Lerp(point3.position, point4.position, lerpValue);
        }

        if (D != null) 
        {
            D.position = Vector3.Lerp(A.position, B.position, lerpValue);
        }
        if (E != null) 
        {
            E.position = Vector3.Lerp(B.position, C.position, lerpValue);
        }

        fillTracePointArray();

        if (Trace != null) 
        {
            previousTracePosition4 = Vector3.Lerp(Vector3.Lerp(Vector3.Lerp(point1.position, point2.position, lerpValue - 0.8f), Vector3.Lerp(point2.position, point3.position, lerpValue - 0.8f), lerpValue - 0.8f), Vector3.Lerp(Vector3.Lerp(point2.position, point3.position, lerpValue - 0.8f), Vector3.Lerp(point3.position, point4.position, lerpValue - 0.8f), lerpValue - 0.8f), lerpValue - 0.8f);
            previousTracePosition3 = Vector3.Lerp(Vector3.Lerp(Vector3.Lerp(point1.position, point2.position, lerpValue - 0.5f), Vector3.Lerp(point2.position, point3.position, lerpValue - 0.5f), lerpValue - 0.5f), Vector3.Lerp(Vector3.Lerp(point2.position, point3.position, lerpValue - 0.5f), Vector3.Lerp(point3.position, point4.position, lerpValue - 0.5f), lerpValue - 0.5f), lerpValue - 0.5f);
            previousTracePosition2 = Vector3.Lerp(Vector3.Lerp(Vector3.Lerp(point1.position, point2.position, lerpValue - 0.3f), Vector3.Lerp(point2.position, point3.position, lerpValue - 0.3f), lerpValue - 0.3f), Vector3.Lerp(Vector3.Lerp(point2.position, point3.position, lerpValue - 0.3f), Vector3.Lerp(point3.position, point4.position, lerpValue - 0.3f), lerpValue - 0.3f), lerpValue - 0.3f);
            previousTracePosition = Vector3.Lerp(Vector3.Lerp(Vector3.Lerp(point1.position, point2.position, lerpValue-0.1f), Vector3.Lerp(point2.position, point3.position, lerpValue-0.1f), lerpValue-0.1f), Vector3.Lerp(Vector3.Lerp(point2.position, point3.position, lerpValue-0.1f), Vector3.Lerp(point3.position, point4.position, lerpValue-0.1f), lerpValue-0.1f), lerpValue - 0.1f);
            Trace.position = Vector3.Lerp(D.position, E.position, lerpValue); // P(t) = (-t^3 + 3t^2 - 3t + 1) + (3t^3 - 6t^2 + 3t) + (-3t^3 + 3t^2) + (t^3)

        }
    }
    private void OnDrawGizmosSelected()
    {
        drawBaseStructure(point1, point2, point3, point4);
        drawBlueLines();
        drawRedLines();
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
        Vector3 currentTracePosition = Trace.position;

        Gizmos.DrawLine(point1.position, previousTracePosition4);
        Gizmos.DrawLine(previousTracePosition4, previousTracePosition3);
        Gizmos.DrawLine(previousTracePosition3, previousTracePosition2);
        Gizmos.DrawLine(previousTracePosition2, previousTracePosition);
        Gizmos.DrawLine(previousTracePosition, currentTracePosition);


    }
}
