ostream &operator<<(ostream & os, const SmectiteIlliteOutput &inOutput)
{
	return os<<inOutput.m_referenceTime<<" "<<inOutput.m_illiteTransfRatio<<endl;
} 
ostream & operator<<(ostream & os, NodeInput &theInput)
{
	return os<<theInput.m_referenceTime<<" "<<theInput.m_temperature<<endl;
} 
