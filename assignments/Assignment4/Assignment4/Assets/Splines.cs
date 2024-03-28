using System.Collections;
using System.Collections.Generic;
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

    public float lerpValue = 0.0f;

    private void Update()
    {
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

        if (Trace != null) 
        {
            Trace.position = Vector3.Lerp(D.position, E.position, lerpValue);
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
            Gizmos.DrawLine(p2.position, p3.position);
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
        if (point1 != null && Trace != null) 
        {
            Gizmos.color = Color.black;
            //Gizmos.DrawLine(point1.position, Trace.position);
        }
    }
}
